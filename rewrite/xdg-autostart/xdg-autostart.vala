// valac xdg-autostart.vala --disable-assert -Xcc "-march=generic -Os"

/*
 * xdg-autostart
 * Copyright (C) 2012,2013 mimas <mimasgpc@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 3.0 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

class Autostart
{
	static string desktop;

	static void
	launch_file (string key, string filename)	{
		string[] show_list;
		string? exec;
		bool found;
		KeyFile kf = new KeyFile();

		try {
			if (kf.load_from_file (filename, KeyFileFlags.NONE)) {
				try {

					message ("Processing %s file.", filename);

					/* Hidden desktop file don't have to be launched */
					if (kf.get_boolean ("Desktop Entry", "Hidden"))
					{
						message ("Hidden attribute set, aborting.");
						return;
					}
				}
				catch (KeyFileError e) {}

				try {
					/* Check if the desktop file is launched in current desktop environment */
					if (kf.has_key ("Desktop Entry", "OnlyShowIn")) {
						show_list = kf.get_string_list ("Desktop Entry", "OnlyShowIn");
						found = false;
						foreach (string de in show_list) {
							if (de == desktop) {
								found = true;
								break;
							}
						}
						/* Current desktop is not found in the OnlyShowIn list */
						if (found == false)
						{
							message ("Not found in OnlyShowIn list, aborting.");
							return;
						}
					}
					/* Check if the desktop file is not launched in current desktop environment */
					else if (kf.has_key ("Desktop Entry", "NotShowIn")) {
						show_list = kf.get_string_list ("Desktop Entry", "NotShowIn");
						foreach (string de in show_list) {
							if (de == desktop) {
								message ("Found in NotShowIn list, aborting.");
								return;
							}
						}
					}

					/* Lookup for TryExec file and check if it's found in path */
					exec = kf.get_string ("Desktop Entry", "TryExec");
					if (exec != null) {
						if (!Path.is_absolute (exec)) {
							if (Environment.find_program_in_path (exec) != null) {
								message ("Can't find %s from TryExec key, aborting.", exec);
								return; // Exec is not found in path => exit
							}
						}
						else if (!FileUtils.test (exec, FileTest.IS_EXECUTABLE))
						{
							message ("%s from TryExec key is not executable, aborting.", exec);
							return; // Exec is not executable => exit
						}
					}

					/* Find the command line to launch and launch it */
					exec = kf.get_string ("Desktop Entry", "Exec");
					try {
						Process.spawn_command_line_async (exec);
						message ("Launching: %s (%s)", exec, key);
					}
					catch (SpawnError e) {
						warning ("Error: %s\n", e.message);
					}
				}
				catch (KeyFileError e) {}
			}
		}
		catch (FileError e) {
			warning ("Error: %s\n", e.message);
		}
	}


	static void get_files_in_dir (HashTable<string, string> table, string directory) {
		unowned string filename;
		string dir_path = Path.build_filename (directory, "autostart");

		try {
			Dir d = Dir.open (dir_path, 0);

			while ((filename = d.read_name ()) != null) {
				if (filename.has_suffix (".desktop")) {
					table.replace (filename, Path.build_filename (dir_path, filename));
				}
			}
		}
		catch (FileError e) {
			warning ("Error: %s\n", e.message);
		}
	}


	static void xdg_autostart () {
		HashTable<string, string> table = new HashTable<string, string> (str_hash, str_equal);

		weak string[] dirs = Environment.get_system_config_dirs ();

		foreach	(string dir in dirs) {
			get_files_in_dir (table, dir);
		}

		get_files_in_dir (table, Environment.get_user_config_dir ());

		if (table.size() > 0) {
			table.for_each (launch_file);
		}
	}

	static int main (string[] args) {
		if (args.length > 1) {
			desktop = args[1];
		}
		else {
			desktop = "OPENBOX";
		}

		xdg_autostart ();
		return 0;
	}
}









