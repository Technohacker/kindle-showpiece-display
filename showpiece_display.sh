#!/bin/sh
# Name: Showpiece Display
# Author: Kevin (Technohacker)
# DontUseFBInk

IMAGE_URL="http://<insert ip address here>"

# How long to wait for the user to exit
TOUCH_WAIT_TIMEOUT=10
# How long to keep the Kindle asleep
SUSPEND_DURATION=60

write_msg() {
	# Simple wrapper around FBInk for showing centered images
	fbink --wait -pmh -y -5 $1
}

exit_on_touch() {
	write_msg "Touch to exit"

	# Wait for a byte from the touchscreen device and stop the script, or
	head -c1 /dev/input/event1 >> /dev/null && kill -SIGTERM "$$" &
	TOUCH_WAIT_PID=$!

	# Wait for a while before continuing as usual
	sleep $TOUCH_WAIT_TIMEOUT &

	# Wait for the timeout to complete
	wait -n

	# If we got here, no touch was detected. Stop the touch wait
	kill $TOUCH_WAIT_PID | true >/dev/null 2>&1
}

suspend_then_wait() {
	# Disable the screensaver first, to allow interrupting device sleep with the power button
	lipc-set-prop com.lab126.powerd preventScreenSaver 1

	# Put the Kindle to sleep
	rtcwake -s $SUSPEND_DURATION

	# And once we're awake again, re-enable the screensaver
	lipc-set-prop com.lab126.powerd preventScreenSaver 0

	# Then allow the user to exit if needed
	exit_on_touch
}

while true; do
	# Clear the whole screen
	fbink --wait --clear

	# Show a progress message for the user
	write_msg "Loading image"
	sleep 1

	# Grab the current frame from the image host, and display it centered horizontally and vertically
	curl --max-time 20 $IMAGE_URL | fbink --wait --clear --flash --image file=-,halign=CENTER,valign=CENTER

	# And put the Kindle to sleep
	suspend_then_wait
done
