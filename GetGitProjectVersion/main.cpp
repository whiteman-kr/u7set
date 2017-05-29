#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <algorithm>
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

stringstream versionInfoText;
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

inline double fileAge(const string& fileName)
{
	struct stat info;

	if(stat(fileName.c_str(), &info) != 0)
	{
		return -1;
	}
	else
	{
		return difftime(time(0), info.st_mtime);
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
			versionInfoText << "#error Git error " << err->klass << ": " << err->message << endl;
		}
		else
		{
			cout << "Git error " << error << ": no detailed info" << endl;
			versionInfoText << "#error Git error " << error << ": no detailed info" << endl;
		}
		return true;
	}
	return false;
}

#define REPORT(a) if (report(a)) cout << "while running "#a << endl;
#define REPORT_RETURN1(a) if (report(a)) { cout << "while running "#a << endl; return 1; }

void print_time(const char* const prefix, const git_time& intime)
{
	char out[32];
	time_t t;

	t = (time_t)intime.time + (intime.offset * 60);	// local time

#ifdef __linux__
	struct tm *intm;
	intm = gmtime(&t);
	strftime(out, sizeof(out), "%F %T", intm);
#elif _WIN32
	struct tm intm;
	gmtime_s(&intm, &t);
	strftime(out, sizeof(out), "%F %T", &intm);
#else
#error Unknown operating system
#endif

	versionInfoText << "#define " << prefix << "_DATE \"" << out << " \"\n";
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

	versionInfoText << "#define " << prefix << "_SHA \"" << sha << "\"\n";

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

	versionInfoText << "#define " << prefix << "_NUMBER " << i << endl;

	git_commit *commit;
	REPORT(git_commit_lookup(&commit, repo, &oid));

	const git_signature *author = git_commit_author(commit);
	versionInfoText << "#define " << prefix << "_AUTHOR \"" << author->name << " " << author->email << "\"\n";
	print_time(prefix, author->when);

	string message = git_commit_message(commit);
	while (message[message.length() - 1] == '\n')
	{
		message.resize(message.size() - 1);
	}
	replaceAll(message, "\n", "\\n\"\\\n\t\"");
	versionInfoText << "#define " << prefix << "_DESCRIPTION \"" << message.c_str() << "\"\n\n";
}

int each_file_cb(const git_diff_delta *delta,
				 float /*progress*/,
				 void* payload)
{
	versionInfoText << "\t\"" << delta->new_file.path << "\",\n";
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

	double versionFileAge = fileAge(versionFileName);

	if (versionFileAge != -1 && versionFileAge < 5)
	{
		cout << versionFileName.c_str() << " checked less than 5 seconds ago, skipping\n";
		return 0;
	}

	versionInfoText << "// Automatically generated file\n"
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

	git_reference* currentReference = NULL;
	string branchName;
	REPORT(git_repository_head(&currentReference, repo));
	if (currentReference != NULL)
	{
		branchName = git_reference_shorthand(currentReference);
		git_reference_free(currentReference);
	}

	int fileCount = -1;

	if (branchName == "HEAD")
	{
		branchName += " is detached";
		versionInfoText << "#define " << "LAST_SERVER_COMMIT_SHA \"unknown\"\n";
		versionInfoText << "#define " << "LAST_SERVER_COMMIT_NUMBER 0\n";
		versionInfoText << "#define " << "LAST_SERVER_COMMIT_AUTHOR \"unknown\"\n";
		versionInfoText << "#define " << "LAST_SERVER_COMMIT_DATE \"unknown\"\n";
		versionInfoText << "#define " << "LAST_SERVER_COMMIT_DESCRIPTION \"unknown\"\n\n";

		versionInfoText << "#define " << "USED_SERVER_COMMIT_SHA \"unknown\"\n";
		versionInfoText << "#define " << "USED_SERVER_COMMIT_NUMBER 0\n";
		versionInfoText << "#define " << "USED_SERVER_COMMIT_AUTHOR \"unknown\"\n";
		versionInfoText << "#define " << "USED_SERVER_COMMIT_DATE \"unknown\"\n";
		versionInfoText << "#define " << "USED_SERVER_COMMIT_DESCRIPTION \"unknown\"\n\n";
	}
	else
	{
		git_oid branch_oid = {0};
		vector<git_oid> branch_history;
		REPORT_RETURN1(git_reference_name_to_id(&branch_oid, repo, ("refs/remotes/origin/"+branchName).c_str()));
		print_commit_info("LAST_SERVER_COMMIT", branch_oid, &branch_history);

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
		for (; commit_index < std::min(branch_history.size(), head_history.size()); commit_index++)
		{
			if (memcmp(branch_history[commit_index].id,head_history[commit_index].id, GIT_OID_RAWSZ) != 0)
			{
				break;
			}
		}
		git_oid used_server_oid = branch_history[commit_index - 1];
		print_commit_info("USED_SERVER_COMMIT", used_server_oid);

		versionInfoText << "const char* const ChangedFilesList[] =\n{\n";

		git_tree *tree = NULL;
		git_diff *diff = NULL;
		git_commit *commit = NULL;
		REPORT(git_commit_lookup(&commit, repo, &used_server_oid));
		REPORT(git_tree_lookup(&tree, repo, git_commit_tree_id(commit)));
		REPORT(git_diff_tree_to_workdir_with_index(&diff, repo, tree, NULL));
		fileCount = 0;
#if defined(_WIN32) || LIBGIT2_VER_MINOR < 23
		REPORT(git_diff_foreach(diff, each_file_cb, nullptr, nullptr, &fileCount));
#else
		REPORT(git_diff_foreach(diff, each_file_cb, nullptr, nullptr, nullptr, &fileCount));
#endif

		if (fileCount == 0)
		{
			versionInfoText << "\t\" \"\n"
							<< "};\n\n"
							<< "const uint CHANGED_FILES_COUNT = 0;\n\n";
		}
		else
		{
			versionInfoText << "};\n\n"
							<< "const uint CHANGED_FILES_COUNT = sizeof(ChangedFilesList) / sizeof(ChangedFilesList[0]);\n\n";
		}
	}

	if (branchName == "master")
	{
		branchName = "stable";
	}
	versionInfoText << "#define BUILD_BRANCH \"" << branchName << "\"\n";

	if (fileCount != 0)
	{
		versionInfoText << "#define BUILD_STATE \"Local build\"\n"
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
		versionInfoText << "#define BUILD_STATE = \"Release build\"\n";
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

	versionInfoText << "#endif\t//GIT_VERSION_FILE\n";

	fstream versionFile;
	versionFile.open(versionFileName);
	string previousVersionInfoText;
	string line;
	while(getline(versionFile, line))
	{
		previousVersionInfoText += line + "\n";
	}
	versionFile.close();

	versionFile.open(versionFileName, std::ofstream::out | std::ofstream::trunc);
	versionFile << versionInfoText.rdbuf();
	versionFile.close();

	if (previousVersionInfoText == versionInfoText.str())
	{
		cout << versionFileName.c_str() << " is same\n";
	}
	else
	{
		cout << versionFileName.c_str() << " has been changed\n";
	}

	return 0;
}

