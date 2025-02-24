import os
import subprocess

installerDir = os.path.dirname(__file__) + "\\bin\\"

#print(installerDir)

files = os.listdir(installerDir)

#print(files)

installers = []

for fileName in files:
    if fileName.startswith("u7setinstall-") and fileName.endswith(".exe"):
        installers.append(fileName)

if len(installers) == 0:
    print("Error: installer not found in " + installerDir)
    exit()

if len(installers) > 1:
    print("Error: more than one installer found in " + installerDir)
    exit()

installer = installers[0]

print("\nFound installer: " + installerDir + installer)

# ----- parse Version

versionStart = installer.find("-", 0)
versionEnd = installer.find("-", versionStart + 1)

if versionStart == -1 or versionEnd == -1 :
    print("Istaller file name parsing error!")
    exit()

version = installer[versionStart + 1:versionEnd]

# ----- parse Pipeline ID

ppidStart = installer.find("PPID_", versionEnd + 1)
ppidEnd = installer.find("-", ppidStart + 5)

if ppidStart == -1 or ppidEnd == -1 :
    print("Istaller file name parsing error!")
    exit()

ppid = installer[ppidStart + 5:ppidEnd]

# ----- parse Release type

releaseTypeStart = installer.find("-", ppidEnd)
releaseTypeEnd = installer.find("-", releaseTypeStart + 1)

if releaseTypeStart == -1 or releaseTypeEnd == -1 :
    print("Istaller file name parsing error!")
    exit()

releaseType = installer[releaseTypeStart + 1:releaseTypeEnd]

# ----- parse Commit SHA

shaStart = installer.rfind("_")
shaEnd = len(installer) - len(".exe")

if shaStart == -1 or shaEnd == -1 :
    print("Istaller file name parsing error!")
    exit()

sha = installer[shaStart + 1:shaEnd]

# ----- parse Branch

branshStart = releaseTypeEnd + 1
branshEnd = shaStart

if branshStart == -1 or branshEnd == -1 :
    print("Istaller file name parsing error!")
    exit()

branch = installer[branshStart:branshEnd]

# -----

if  len(version) == 0 or len(ppid) == 0 or len(releaseType) == 0 or len(branch) == 0 or len(sha) == 0:
    print("Istaller file name parsing error!")
    exit()

print("\n")
print("Version:      " + version)
print("Pipeline ID:  " + ppid)
print("Release type: " + releaseType)
print("Branch:       " + branch)
print("Commit SHA:   " + sha)
print("\n")

# set environement variables

os.environ["RPCT_VERSION"] = version
os.environ["U7SET_FULL_VERSION"] = version
os.environ["CI_PIPELINE_ID"] = ppid
os.environ["CI_RELEASE_TYPE"] = releaseType
os.environ["CI_COMMIT_REF_SLUG"] = branch
os.environ["CI_COMMIT_SHA"] = sha

subprocess.run("windows_code_sign.bat " + ppid)