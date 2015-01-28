#include <iostream>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <git2/errors.h>
#include <git2/threads.h>
#include <git2/repository.h>
#include <git2/revwalk.h>
#include <git2/commit.h>
#include <git2/refs.h>
#include <git2/diff.h>
#include <git2/revparse.h>

using namespace std;

ofstream versionFile;
git_repository *repo = nullptr;

inline bool fileExists(const string& fileName)
{
	if (FILE *file = fopen(fileName.c_str(), "r"))
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
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

void print_time(const char* const prefix, const git_time& intime)
{
	char sign, out[32];
	struct tm *intm;
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

	intm = gmtime(&t);
	strftime(out, sizeof(out), "%a %b %e %T %Y", intm);

	versionFile << "#define " << prefix << "_DATE \"" << out << ' '
		 << sign << setw(2) << setfill('0') << hours
		 << setw(2) << minutes << "\"\n";
}

void myReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
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
	report(git_revwalk_new(&walker, repo));
	git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
	report(git_revwalk_push(walker, &oid));

	git_oid walked_oid;
	int i = 0;
	while (!git_revwalk_next(&walked_oid, walker))
	{
		if (history)
		{
			history->insert(history->cbegin(), walked_oid);
		}
		i++;
	}

	versionFile << "#define " << prefix << "_NUMBER " << i << endl;

	git_commit *commit;
	report(git_commit_lookup(&commit, repo, &oid));

	const git_signature *author = git_commit_author(commit);
	versionFile << "#define " << prefix << "_AUTHOR \"" << author->name << " " << author->email << "\"\n";
	print_time(prefix, author->when);

	string message = git_commit_message(commit);
	while (message[message.length() - 1] == '\n')
	{
		message.resize(message.size() - 1);
	}
	myReplace(message, "\n", "\"\\\n\t\"");
	versionFile << "#define " << prefix << "_DESCRIPTION \"" << message << "\"\n\n";
}

int each_file_cb(const git_diff_delta *delta,
				 float /*progress*/,
				 void */*payload*/)
{
	versionFile << "\t\"" << delta->new_file.path << "\",\n";
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
		cout << "Project file is not under git control";
		return 1;
	}

	string versionFileName = projectDir + "/version.h";

	versionFile.open(versionFileName.c_str());
	versionFile << "// Automatically generated file\n"
				<< "// Parameters:\n"
				<< "// \tProjectPath: " << projectDir << "\n"
				<< "// \tProjectFileName: " << projectFileName << "\n"
				<< "//\n\n"
				<< "#ifndef GIT_VERSION_FILE\n"
				<< "#define GIT_VERSION_FILE\n\n";

	report(git_threads_init());
	report(git_repository_open(&repo, dir.c_str()));

	git_oid master_oid = {0};
	vector<git_oid> master_history;
	if (report(git_reference_name_to_id(&master_oid, repo, "refs/remotes/origin/master")))
	{
		return -1;
	}
	print_commit_info("SERVER_COMMIT", master_oid, &master_history);

	git_oid head_oid = {0};
	vector<git_oid> head_history;
	if (report(git_reference_name_to_id(&head_oid, repo, "HEAD")))
	{
		return -1;
	}
	print_commit_info("LOCAL_COMMIT", head_oid, &head_history);

	uint commit_index = 0;
	for (; commit_index < min(master_history.size(), head_history.size()); commit_index++)
	{
		if (memcmp(master_history[commit_index].id,head_history[commit_index].id, GIT_OID_RAWSZ) != 0)
		{
			break;
		}
	}
	print_commit_info("LAST_COMMON_COMMIT", master_oid);

	versionFile << "const char* const ChangedFilesList[] =\n{\n";

	git_object *obj = NULL;
	if (!report(git_revparse_single(&obj, repo, "HEAD^{tree}")))
	{
		git_tree *tree = NULL;
		if (!report(git_tree_lookup(&tree, repo, git_object_id(obj))))
		{
			git_diff *diff = NULL;
			if (!report(git_diff_tree_to_workdir_with_index(&diff, repo, tree, NULL)))
			{
				report(git_diff_foreach(diff, each_file_cb, nullptr, nullptr, nullptr));
			}
		}
	}

	versionFile << "};\n\n"
				<< "const uint CHANGED_FILES_COUNT = sizeof(ChangedFilesList) / sizeof(ChangedFilesList[0]);\n\n";

	if (repo)
	{
		git_repository_free(repo);
	}
	git_threads_shutdown();

	versionFile << "#endif\t//GIT_VERSION_FILE\n";

	versionFile.close();

	cout << versionFileName << " was generated\n";

	return 0;
}

