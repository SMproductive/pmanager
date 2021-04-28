#!/bin/bash
ls $HOME/.pmanager/ |dmenu |xargs pmanager -get |xclip -l 1 -selection clipboard
