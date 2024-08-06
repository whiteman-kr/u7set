# Creates variables:
#	U7SET_MAJOR_VERSION, 
#	U7SET_MINOR_VERSION, 
#	U7SET_PATCH_VERSION, 
#	U7SET_BRANCH_NAME,
#	U7SET_RELEASE_TYPE
#

# set U7SET_RELEASE_TYPE to Default/Stable,
# if environment variable CI_STABLE_RELEASE is not defined, set it to "Default".
#
if (DEFINED ENV{CI_RELEASE_TYPE})
	if ($ENV{CI_RELEASE_TYPE} STREQUAL "Stable")
		set(U7SET_RELEASE_TYPE "Stable")
		set(U7SET_IS_DEFAULT_RELEASE 0)
		set(U7SET_IS_STABLE_RELEASE 1)
	else()
		set(U7SET_RELEASE_TYPE "Default")
		set(U7SET_IS_DEFAULT_RELEASE 1)
		set(U7SET_IS_STABLE_RELEASE 0)
	endif()
else()
	set(U7SET_RELEASE_TYPE "Default")
	set(U7SET_IS_DEFAULT_RELEASE 1)
	set(U7SET_IS_STABLE_RELEASE 0)
endif()

# Major version is set from the CI.
# 
if (DEFINED ENV{CI_MAJOR_VERSION})
	set(U7SET_MAJOR_VERSION $ENV{CI_MAJOR_VERSION})
else()
	set(U7SET_MAJOR_VERSION 0)
endif()

# U7SET_MINOR_VERSION can be set by CI pipeline, if it was not defined, set it to 0.
#
if (DEFINED ENV{CI_MINOR_VERSION})
	set(U7SET_MINOR_VERSION $ENV{CI_MINOR_VERSION})
else()
	set(U7SET_MINOR_VERSION 0)
endif()

# Patch version, for Default releases always is pipeline id, 
# for Stable releases it should be defined by environment variable U7SET_PATCH_VERSION.
#
if (U7SET_IS_STABLE_RELEASE EQUAL 1)
	# For Stable releases patch version should be defined by environment variable U7SET_PATCH_VERSION.
	#
	if (NOT DEFINED ENV{CI_PATCH_VERSION})
		set(U7SET_PATCH_VERSION 999999)
		message("U7SET_PATCH_VERSION is not defined for Stable release. Forced to 999999.")
	else()
		set(U7SET_PATCH_VERSION $ENV{CI_PATCH_VERSION})
	endif()
else()
	# For Default releases minor version is a pipeline id.
	#
	if(DEFINED ENV{CI_PIPELINE_ID})
		set(U7SET_PATCH_VERSION $ENV{CI_PIPELINE_ID})
	else()
		set(U7SET_PATCH_VERSION 999999)
	endif()
endif()

# Forming current branch name
# if environment variable CI_COMMIT_REF_NAME is not defined, set it to "Unknown".
#
if(DEFINED ENV{CI_COMMIT_REF_SLUG})
	set(U7SET_BRANCH_NAME $ENV{CI_COMMIT_REF_SLUG})
else()
	set(U7SET_BRANCH_NAME "Unknown")
endif()

# Forming current commit hash
# if environment variable CI_COMMIT_SHA is not defined, set it to "Unknown".
#
if(DEFINED ENV{CI_COMMIT_SHA})
	set(U7SET_COMMIT_HASH $ENV{CI_COMMIT_SHA})
else()
	set(U7SET_COMMIT_HASH "Unknown")
endif()

# Forming current commit author
# if environment variable GITLAB_USER_NAME is not defined, set it to "Unknown".
#
if(DEFINED ENV{$GITLAB_USER_NAME})
	set(U7SET_USER_NAME $ENV{GITLAB_USER_NAME})
else()
	if(WIN32)
		set(U7SET_USER_NAME $ENV{USERNAME})
	else(UNIX)
		set(U7SET_USER_NAME $ENV{USER})
	endif()
endif()

# Set U7SET_BUILD_DATE to current date and time in the format YYYY-MM-DD HH:MM:SS
#
string(TIMESTAMP U7SET_BUILD_DATE "%Y-%m-%d %H:%M:%S")
string(TIMESTAMP U7SET_BUILD_DATE_SECONDS "%s") # Seconds since 1970-01-01 00:00:00 UTC

# Set U7SET_HOSTNAME to current hostname
#
cmake_host_system_information(RESULT U7SET_HOSTNAME QUERY HOSTNAME)

# set U7SET_PIPELINE_ID to current pipeline id, if not defined, set it to 0.
#
if(DEFINED ENV{CI_PIPELINE_ID})
	set(U7SET_PIPELINE_ID $ENV{CI_PIPELINE_ID})
else()
	set(U7SET_PIPELINE_ID 0)
endif()

# Configure the version.h
#
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

# Set variable U7SET_FULL_VERSION to full version string.
#
set(U7SET_FULL_VERSION ${U7SET_MAJOR_VERSION}.${U7SET_MINOR_VERSION}.${U7SET_PATCH_VERSION})