# Fullscreen Blocker Setup Guide

This guide will walk you through the process of setting up the `fullscreen_blocker` application, ensuring it runs as a user-level `systemd` service and is managed by cron jobs. The application will run within the user's graphical session.

## Compilation

1. Ensure you have the necessary development tools and libraries installed:
   ```bash
   sudo zypper install gcc make libX11-devel libXinerama-devel
   ```

2. Compile the application:
   ```bash
   gcc fullscreen_blocker.c -o fullscreen_blocker -lX11 -lXinerama -lpthread
   ```

3. Copy the compiled binary 
   ```bash
   mkdir -p ~/bin
   mv fullscreen_blocker ~/bin/fullscreen_blocker
   ```

### Systemd Service Setup

Create the directory for user-level systemd services if it does not already exist:
   ``` bash
   mkdir -p ~/.config/systemd/user
   ```

Create the fullscreen_blocker service file:
   ```bash
   cp fullscreen_blocker.service ~/.config/systemd/user/fullscreen_blocker.service
   ```

Reload the user systemd manager configuration:
   ```bash
   systemctl --user daemon-reload
   ```
    
Add to the user crontab:

```bash
crontab -e

# Add the following lines to start and stop the service at the specified times:

# Start the service every day at 1:30
30 1 * * * systemctl --user start fullscreen_blocker

# Stop the service every day at 6:30
30 6 * * * systemctl --user stop fullscreen_blocker
```
