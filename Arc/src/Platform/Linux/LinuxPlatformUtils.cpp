#include "arcpch.h"

#ifdef ARC_PLATFORM_LINUX

#include "Arc/Utils/PlatformUtils.h"

#include <gtk/gtk.h>

namespace ArcEngine
{
	std::string FileDialogs::OpenFolder()
	{
		GtkWidget* dialog;
		int res;

		dialog = gtk_file_chooser_dialog_new("Open", nullptr, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);

		res = gtk_dialog_run(GTK_DIALOG(dialog));
		std::string ret;
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char* filename;
			GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
			filename = gtk_file_chooser_get_filename(chooser);
			ret = filename;
			g_free(filename);
		}

		gtk_widget_destroy(dialog);
		return ret;
	}

	std::string FileDialogs::OpenFile([[maybe_unused]] const char* filter)
	{
		GtkWidget* dialog;
		int res;

		if (!gtk_init_check(NULL, NULL) )
		{
			return "";
		}

		dialog = gtk_file_chooser_dialog_new("Open File", nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);

		res = gtk_dialog_run(GTK_DIALOG(dialog));
		std::string ret;
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char* filename;
			GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
			filename = gtk_file_chooser_get_filename(chooser);
			ret = filename;
			g_free(filename);
		}

		gtk_widget_destroy(dialog);
		return ret;
	}

	std::string FileDialogs::SaveFile([[maybe_unused]] const char* filter)
	{
		GtkWidget* dialog;
		GtkFileChooser* chooser;
		int res;

		dialog = gtk_file_chooser_dialog_new("Save File", nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, nullptr);
		chooser = GTK_FILE_CHOOSER(dialog);

		gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
		gtk_file_chooser_set_current_name(chooser, "Untitled document");
		res = gtk_dialog_run(GTK_DIALOG(dialog));
		std::string ret;
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char* filename = gtk_file_chooser_get_filename(chooser);
			ret = filename;
			g_free(filename);
		}

		gtk_widget_destroy(dialog);
		return ret;
	}

	void FileDialogs::OpenFolderAndSelectItem(const char* path)
	{
		std::string cmd = "xdg-open ";
		cmd += path;
		std::system(cmd.c_str());
	}

	void FileDialogs::OpenFileWithProgram(const char* path)
	{
		OpenFolderAndSelectItem(path);
	}
}

#endif
