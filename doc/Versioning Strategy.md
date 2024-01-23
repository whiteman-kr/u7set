# Versioning Strategy for u7set

## Overview

The versioning strategy for the "u7set" software involves managing releases through two distinct types: **Default** and **Stable**. This document outlines the processes and practices associated with versioning for the project.

## Release Types

### Default Releases

Default releases are derived from the 'develop' branch in the Git repository. These releases serve as snapshots of the ongoing development process and are not intended for production use. 

Default releases do not receive updates once published.

Default releases are identified by a version number with an odd Major value and are generated automatically based on the GitLab PipelineID for the Patch component.

Example: `11.0.<PipelineID>`

### Stable Releases

Stable releases are managed through dedicated branches in the Git repository. 

Unlike Default releases, Stable releases can receive updates, including bug fixes and additional features. Stability is a key focus for these releases, making them suitable for production environments.

Stable releases are identified by a version number with an even Major value.

Example: `10.0.1`

After releasing a major Stable version for the first time, the Default release also increments the Major version to the Stable Version + 1. Subsequent Default releases will then have an odd Major version.

## Digital Signatures

All **Stable** releases are digitally signed to ensure the integrity and authenticity of the software distribution.

## Examples

- Version 10.0.0 - Stable release
- Version 11.0.3000 - Default release
- Version 11.0.3010 - Default release
- Version 11.0.3020 - Default release
- Version 12.0.0 - Stable release
- Version 13.0.3055 - Default release
- ...

## Conclusion

This versioning strategy provides a structured approach to managing releases in the "u7set" project, distinguishing between Default and Stable releases, and ensuring version numbers convey important information about the nature of the release.

-------------------------------

10 - 5.0
11 - develop
12 - 5.1
13 - develop


u7 v0.10.1 (release-ph-5.0)

Pipeline: #2791
Build: Debug
Commit SHA1: No data
Branch: No data
Date: No data

