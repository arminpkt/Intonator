#!/bin/bash
# Post-install for the "Ableton Extension" package.
# Runs as root -> copy into the logged-in user's User Library, AS that user.

SRC="/Library/Application Support/UnTETered/UnTETeredSync"

# The user actually sitting at the Mac (not root)
CONSOLE_USER=$(stat -f%Su /dev/console)
if [ -z "$CONSOLE_USER" ] || [ "$CONSOLE_USER" = "root" ]; then
    echo "No logged-in user, skipping Ableton extension"
    exit 0
fi

USER_HOME=$(dscl . -read "/Users/$CONSOLE_USER" NFSHomeDirectory | awk '{print $2}')
DEST="$USER_HOME/Music/Ableton/User Library/Remote Scripts/UnTETered"

# Remove a stale Python cache from an older version
rm -rf "$DEST/__pycache__"

# ditto creates missing folders; running it as the user keeps ownership correct
sudo -u "$CONSOLE_USER" ditto "$SRC" "$DEST"

exit 0