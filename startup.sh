#!/bin/sh

echo "[`date`] Starting OpenStratos..." >> /root/control.log
/root/openstratos >> /root/control.log 2>&1 &
sleep 5

while true; do
	(pgrep procname && sleep 60) ||
	(echo "[`date`] Process not running, restarting..." >> /root/control.log; shutdown -r now)
done
