#!/bin/bash
ls $HOME/.pmanager/ |dmenu -b -fn "Open Sans:size=18" -nb "#2e3440" -nf "#d8dee9" -sb "#434c5e" -sf "#eceff4" |xargs pmanager -get
