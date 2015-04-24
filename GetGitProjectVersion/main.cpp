#include <iostream>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <git2/errors.h>
#include <git2/version.h>

#ifdef _WIN32
#include <git2/global.h>
#elif !LIBGIT2_VER_MAJOR && LIBGIT2_VER_MINOR > 21
#include <git2/global.h>
#else
#include <git2/threads.h>
#define GIT2_THREADS_INIT
#endif

#include <git2/repository.h>
#include <git2/revwalk.h>
#include <git2/commit.h>
#include <git2/refs.h>
#include <git2/diff.h>
#include <git2/revparse.h>

using namespace std;

ofstream versionFile;
git_repository *repo = nullptr;

#ifdef _WIN32
template <typename TYPE>
inline TYPE min(TYPE a, TYPE b)
{
	if (a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
}
#endif

inline bool fileExists(const string& fileName)
{
	struct stat info;

	if(stat(fileName.c_str(), &info) != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool report(int error)
{
	if (error != 0)
	{
		const git_error *err = giterr_last();
		if (err)
		{
			cout << "Git error " << err->klass << ": " << err->message << endl;
			versionFile << "#error Git error " << err->klass << ": " << err->message << endl;
		}
		else
		{
			cout << "Git error " << error << ": no detailed info" << endl;
			versionFile << "#error Git error " << error << ": no detailed info" << endl;
		}
		return true;
	}
	return false;
}

#define REPORT(a) if (report(a)) cout << "while running "#a << endl;
#define REPORT_RETURN1(a) if (report(a)) { cout << "while running "#a << endl; return 1; }

void print_time(const char* const prefix, const git_time& intime)
{
	char sign, out[32];
	int offset, hours, minutes;
	time_t t;

	offset = intime.offset;
	if (offset < 0) {
		sign = '-';
		offset = -offset;
	} else {
		sign = '+';
	}

	hours   = offset / 60;
	minutes = offset % 60;

	t = (time_t)intime.time + (intime.offset * 60);

#ifdef __linux__
	struct tm *intm;
    intm = gmtime(&t);
	strftime(out, sizeof(out), "%a %b %e %T %Y", intm);
#elif _WIN32
	struct tm intm;
    gmtime_s(&intm, &t);
	strftime(out, sizeof(out), "%a %b %d %X %Y", &intm);
#else
#error Unknown operating system
#endif
	versionFile << "#define " << prefix << "_DATE \"" << out << ' '
				<< sign << setw(2) << setfill('0') << hours
				<< setw(2) << minutes << "\"\n";
}

void replaceAll(std::string& str, const std::string& oldStr, const std::string& newStr)
{
	size_t pos = 0;
	while((pos = str.find(oldStr, pos)) != std::string::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}

void print_commit_info(const char* const prefix, const git_oid oid, vector<git_oid>* history = nullptr)
{
	char sha[41] = {0};
	git_oid_tostr(sha, 40, &oid);

	versionFile << "#define " << prefix << "_SHA \"" << sha << "\"\n";

	git_revwalk *walker;
	REPORT(git_revwalk_new(&walker, repo))
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
	REPORT(git_revwalk_push(walker, &oid));

	git_oid walked_oid;
	int i = 0;
	while (!git_revwalk_next(&walked_oid, walker))
	{
		if (history)
		{
			history->insert(history->begin(), walked_oid);
		}
		i++;
	}

	versionFile << "#define " << prefix << "_NUMBER " << i << endl;

	git_commit *commit;
	REPORT(git_commit_lookup(&commit, repo, &oid));

	const git_signature *author = git_commit_author(commit);
	versionFile << "#define " << prefix << "_AUTHOR \"" << author->name << " " << author->email << "\"\n";
	print_time(prefix, author->when);

	string message = git_commit_message(commit);
	while (message[message.length() - 1] == '\n')
	{
		message.resize(message.size() - 1);
	}
	replaceAll(message, "\n", "\\n\"\\\n\t\"");
	versionFile << "#define " << prefix << "_DESCRIPTION \"" << message.c_str() << "\"\n\n";
}

int each_file_cb(const git_diff_delta *delta,
				 float /*progress*/,
				 void* payload)
{
	versionFile << "\t\"" << delta->new_file.path << "\",\n";
	*(int*)payload += 1;
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "You should pass pro-file path as an argument.\n" << endl;
		return 1;
	}

	string dir = argv[1];
	//string dir = "/home/vsapronenko/GitData/u7set/u7/u7.pro";
	//string dir = "D:/GitData/u7set/ServiceControlManager/ServiceControlManager.pro";
	if (!fileExists(dir))
	{
		cout << "Project file doesn't exists" << endl;
		return 1;
	}

	size_t slashPos = dir.rfind('/');
	string projectFileName = dir.substr(slashPos + 1, string::npos);
	string projectDir = dir = dir.substr(0, slashPos);
	while (!fileExists(dir + "/.git") && ((slashPos = dir.rfind('/')) != string::npos))
	{
		dir = dir.substr(0, slashPos);
	}
	if (slashPos == string::npos)
	{
		cout << "Project file is not under git control\n";
		return 1;
	}

	string versionFileName = projectDir + "/version.h";

	versionFile.open(versionFileName.c_str());
	versionFile << "// Automatically generated file\n"
				<< "// Parameters:\n"
				<< "// \tProjectPath: " << projectDir.c_str() << "\n"
				<< "// \tProjectFileName: " << projectFileName.c_str() << "\n"
				<< "//\n\n"
				<< "#ifndef GIT_VERSION_FILE\n"
				<< "#define GIT_VERSION_FILE\n\n";

#ifndef GIT2_THREADS_INIT
	git_libgit2_init();
#else
	REPORT(git_threads_init());
#endif
	REPORT(git_repository_open(&repo, dir.c_str()));

	git_oid master_oid = {0};
	vector<git_oid> master_history;
	REPORT_RETURN1(git_reference_name_to_id(&master_oid, repo, "refs/remotes/origin/master"));
	print_commit_info("LAST_SERVER_COMMIT", master_oid, &master_history);

	// Fill local local commit history vector
	//
	git_oid head_oid = {0};
	vector<git_oid> head_history;
	REPORT_RETURN1(git_reference_name_to_id(&head_oid, repo, "HEAD"));
	git_revwalk *walker;
	REPORT(git_revwalk_new(&walker, repo))
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
	REPORT(git_revwalk_push(walker, &head_oid));

	git_oid walked_oid;
	while (!git_revwalk_next(&walked_oid, walker))
	{
		head_history.insert(head_history.begin(), walked_oid);
	}
	//

	unsigned int commit_index = 0;
	for (; commit_index < min(master_history.size(), head_history.size()); commit_index++)
	{
		if (memcmp(master_history[commit_index].id,head_history[commit_index].id, GIT_OID_RAWSZ) != 0)
		{
			break;
		}
	}
	git_oid used_server_oid = master_history[commit_index - 1];
	print_commit_info("USED_SERVER_COMMIT", used_server_oid);

	versionFile << "const char* const ChangedFilesList[] =\n{\n";

	git_tree *tree = NULL;
	git_diff *diff = NULL;
	git_commit *commit = NULL;
	REPORT(git_commit_lookup(&commit, repo, &used_server_oid));
	REPORT(git_tree_lookup(&tree, repo, git_commit_tree_id(commit)));
	REPORT(git_diff_tree_to_workdir_with_index(&diff, repo, tree, NULL));
	int fileCount = 0;
	REPORT(git_diff_foreach(diff, each_file_cb, nullptr, nullptr, &fileCount));

	if (fileCount == 0)
	{
		versionFile << "\t\" \"\n"
					<< "};\n\n"
					<< "const uint CHANGED_FILES_COUNT = 0;\n\n";
	}
	else
	{
		versionFile << "};\n\n"
					<< "const uint CHANGED_FILES_COUNT = sizeof(ChangedFilesList) / sizeof(ChangedFilesList[0]);\n\n";
	}

	if (fileCount > 0)
	{
		versionFile << "#define BUILD_STATE = \"Local build\"\n"
					<< "#ifndef Q_DEBUG\n"
					<< "#ifdef _MSC_VER\n"
					<< " #pragma warning ()\n"
					<< " #pragma message (\" --- Local build is used, push changes in git repository and build release --- \")\n"
					<< "#else\n"
					<< " #warning  --- Local build is used, push changes in git repository and build release --- \n"
					<< "#endif\t//_MSC_VER\n"
					<< "#endif\t//NDEBUG\n";
	}
	else
	{
		versionFile << "#define BUILD_STATE = \"Release build\"\n";
	}

	if (repo)
	{
		git_repository_free(repo);
	}
#ifndef GIT2_THREADS_INIT
	git_libgit2_shutdown();
#else
	git_threads_shutdown();
#endif

	versionFile << "#endif\t//GIT_VERSION_FILE\n";

	versionFile.close();

	cout << versionFileName.c_str() << " was generated\n";

	return 0;
}

