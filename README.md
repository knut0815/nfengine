nfEngine README
===============

nfEngine is an open source 3D game engine.

*TODO: fill this section*

Contents
--------

This document consists of following sections (in order of appearance in file):

- Dependencies
- Documentation
- Formatting
- Issue tracking
- Workflow and branching
- Commit message format
- Pull request description format

Dependencies
------------

To make the code compileable, the following requirements have to be met:

1. Installed Visual Studio 2013.
2. Installed DirectX SDK.
3. Downloaded external dependencies from [here](https://drive.google.com/drive/u/0/#folders/0B66mya2agFOEd0RJUWx1aDZ6Ym8)
    * **"Deps"** directory created in repository root with content copied from subfolder **nfEngineDeps**
    * **"Data"** directory created in **"nfEngineTest"** with content copied from subfolder **nfEngineTestData**
    * **NOTE:** it is convenient to use Google Drive application to synchronize these folders automatically (by adding this folder to your drive) and create symbolic links to them:
            - on Windows: "mklink /J <dest> <src>"
            - on Linux: "ln -s <src> <dest>"

Example code on Windows:

```
cd nfengine // go to root of repository
mklink /J Deps "path-to-nfEngineDeps"
cd nfEngineTest
mklink /J Data "path-to-nfEngineTestData"
```

Documentation
-------------

Doxygen documentation can be generated using "gen_doc.bat" script. Doxygen needs to be installed in the system and "doxygen" command must be visible in the shell.

Formatting
----------

Keep the code format consistent. Run format.bat script to format the code automatically. ArtisticStyle needs to be installed in the system and "astyle" command must be visible in the shell.

ArtisticStyle can be downloaded [here](http://astyle.sourceforge.net/).

Basic coding guidelines:

* Code is written in Allman style.
* We use spaces to indent parts of code. Each indentation takes 4 spaces. Mixed tabs and spaces are forbidden.
* To make code reviewing easier, keep max 100 chars per line. Exceptions to that rule might occur if it is absolutely necessary, depending on the situation (keep in mind, some legacy code in the repository might not follow this rule).
* Each source file should have a doxygen boilerplate which will briefly describe what this module does. This rule applies to header files as well.
* Use two newlines to separate chunks of code in one file (eg. to split constants from function/method definitions, to split boilerplate from rest of the file, or to split namespaces).

Issue tracking
--------------

Working on nfEngine revolves around GitHub and its issue tracking system. You can find all current issues at [our GitHub Issue Tracker](https://github.com/nfprojects/nfengine/issues).

Usually issues are added by owners and categorized according with three labels:

1. **Type of issue:**
    * bug
    * enhancement
    * new feature
2. **Priority (how important is this issue for the project right now):**
    * low priority
    * medium priority
    * high priority
3. **Estimated complexity of the issue:**
    * small
    * medium
    * huge

Additionall rules applied to labelling issues:

1. Issues labeled as **"new feature"** or **"enhancement"** label can additionally have a **"proposal"** label. Such issues are most probably not completely defined, might be a subject to change and should not be assigned to anyone until **"proposal"** label disappears. Removal of **"proposal"** will happen only after a discussion with project owners, or after completion of issue.
2. Issues labeled with **"bug"** label don't need to be tagged with estimate label if reporter cannot perform such estimation.

When a bug is found, it should be reported on issue tracking system with only **"bug"** label. Such issue should contain detailed information on how to reproduce the bug. After verification of the issue, one of the owners will further process the issue (either by prioritizing it and assigning it to someone, or by closing it if issue is invalid).

Issues crucial to project, usually related to one specific goal, are categorized into milestones. Milestones don't have specific due date. Issues not assigned to milestones are mostly general, small issues (most of these are probably bugs).

Workflow and branching
----------------------

Working on nfEngine revolves around "long-running branches" Git workflow. See [Pro Git book](http://git-scm.com/book/en/Git-Branching-Branching-Workflows) for more info. 

nfEngine project contains two main long-running branches with code:

* **Branch "master"** - here is kept latest stable version of nfEngine, guaranteed to work. On this branch no new changes are committed - it should contain only merge commits with version change.
* **Branch "devel"** - this branch is main development branch used to contribute new changes.

Each change (related to issue in GitHub) should have its own branch on which all changes should be kept. After committing a change, contributor should create a pull request to devel branch and begin code review process this way. After certain conditions are met, changes are merged to devel branch, and in upcoming release to master branch.

Owners and trusted contributors can work directly on main nfEngine repository. Contributors new to the project must fork the repository. Contributor should create a pull request to devel branch after resolving an issue.

When working on an issue, make sure your remote branch name relates to resolved issue in some way. In short terms:

* Branch names like **"aaa"**, **"temp"**, **"work"**, **"devel2"** etc. don't tell much about the issue and pull requests from such branches will be closed without reviewing the change.
* Branch names like **"fix-memleaks"**, **"add-threadpool-test"**, **"update-readme"** etc. are more than welcome and help owners of repository to get around what such change is about. Though, make sure the branch name is not too long.
* In some situations (especially when branch name might be too long) contributor can name a feature branch according to **"issue-X"** scheme, where X is the number of the issue.

Pull request will be merged only when it will pass code review and verification process, and when provided change will be mergeable to devel branch without conflicts.

Direct commits to devel and master branch are forbidden and will be deleted immediately by project owners.

Large changes (more than 500 lines affected) should be split in multiple commits to make review process easier.


Commit message format
---------------------

Commit messages should keep a simple, standard format. Title (first line of commit message) should describe briefly what change does. Then contributor should write more detailed description of committed change. Example commit message is provided below:

```
Fix memory leaks in nfCommon Math module

This commit fixes memory leaks which occured when using NFE::Common::Vector. Leaks
occured due to not freed memory in Vector destructor.
```


Pull request description format
-------------------------------

After finishing a change a pull request should be properly described. Description of pull request should be formatted in commit message format, however with two major changes:

* Contributor should add **[#X]** field to title of pull request and replace X with issue number related to created pull request.
* Contributor should add information to pull request about verification of created change. Such information should give detailed instruction to reviewers what should be done to test the change.
  
Example pull request description:

```
[#00] Improvements to nfCommon Math module

This change improves some of Math module functions in nfCommon:
  * Vector performance is sped up on some CPUs by using AVX instruction set
  * Matrix memory management is optimized`

Verification:
Build, run nfCommonTest on CPU with AVX instructions - there should be new test
category "nfCommonMathAVXTest", all tests should pass.
```
