#!/bin/zsh

source ~/.zshrc
git diff | sgpt-git "Generate git commit message, for my changes" | tee ./tmp/git_commit.txt