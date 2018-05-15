# CSC474Final
Animation project for CSC474 at Cal Poly San Luis Obispo




Dreams-VR

## GIT commands
### From the beginning
- git clone https://github.com/nshockwa/CSC474Final.git
  - (by default, you will be on the master branch)
- git branch 
  - (this should show you all your branches and which branch you are on)
- git checkout -b NAME_HERE
  - (this will create a new branch called NAME_HERE)
- git checkout existing_branch
  - (this will switch you to an existing branch, like paula)

### Each time you develop
- git branch
  - (view all branches)
- git checkout YOUR_BRANCH
- git pull origin master (OPTIONAL) 
  - this will get all the most recent changes from everybody else
- MAKE CHANGES // DO YOUR DEVELOPER THANG
- git add .
  - (this will add all the changes to be ready to commit, but it is not official yet.)
- git status 
  - (this will show you all the changes you made, just good to check.)
- git commit -m "MESSAGE HERE"
  - (this will officially 'check in' your changes so they are tagged like this so other people can see after you push)
- git push origin YOUR_BRANCH
  - (this will push your local commit to your branch remotely)
- REPEAT THE PROCESS

### If things go wrong
- if you have not done "git add .", then you can do the following: 
  - git checkout .
  - this will remove all changes
- if you have done "git add ." but have not done "git commit -m 'MESSAGE HERE'"
  - git reset HEAD . (this will unstage it)
  - git checkout . (this will remove all changes)
- if you have done "git commit -m 'MESSAGE HERE'" 
  - git revert DESIRED_COMMIT_ID
