# What are these programs for?

  * obsession-logout displays a dialog where you can choose if you want
    to quit the session power off, suspend or hibernate the computer.

  * obsession-exit, this is the command line version of obsession-logout.

# Why?

I'm a big fan of Openbox and LXDE, but I don't need all the features of the
last one. So starting from LXDE project and keeping what is useful (for me)
was a good start to have a very lightweight desktop easy to manage.

# This is so old school!

Yes but it works with ConsoleKit, UPower and systemd.

# Configuration

the programs obsession-logout and obsession-exit uses a configuration file
named obsession.conf. This file is automatically created in the .config
directory located in the user home.

The key `screenlock` sets the screen lock command. The key `logout` defines
the logout command. Their respective default values are `xlock -mode blank`
and `openbox --exit`.

