/*
    Copyright (C) 2012 Paul Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <algorithm>
#include <cstring>

#include <glibmm.h>

#include "pbd/basename.h"
#include "pbd/file_utils.h"
#include "pbd/stl_delete.h"
#include "pbd/xml++.h"

#include "ardour/template_utils.h"
#include "ardour/directory_names.h"
#include "ardour/filesystem_paths.h"
#include "ardour/filename_extensions.h"
#include "ardour/search_paths.h"
#include "ardour/io.h"

using namespace std;
using namespace PBD;

namespace ARDOUR {

std::string
user_template_directory ()
{
	return Glib::build_filename (user_config_directory(), templates_dir_name);
}

std::string
user_route_template_directory ()
{
	return Glib::build_filename (user_config_directory(), route_templates_dir_name);
}

static bool
template_filter (const string &str, void* /*arg*/)
{
	if (!Glib::file_test (str, Glib::FILE_TEST_IS_DIR)) {
		return false;
	}

	return true;
}

static bool
route_template_filter (const string &str, void* /*arg*/)
{
	if (str.find (template_suffix) == str.length() - strlen (template_suffix)) {
		return true;
	}

	return false;
}

string
session_template_dir_to_file (string const & dir)
{
	return Glib::build_filename (dir, Glib::path_get_basename(dir) + template_suffix);
}


void
find_session_templates (vector<TemplateInfo>& template_names)
{
	vector<string> templates;

	find_paths_matching_filter (templates, template_search_path(), template_filter, 0, true, true);

	if (templates.empty()) {
		cerr << "Found nothing along " << template_search_path().to_string() << endl;
		return;
	}

	cerr << "Found " << templates.size() << " along " << template_search_path().to_string() << endl;

	for (vector<string>::iterator i = templates.begin(); i != templates.end(); ++i) {
		string file = session_template_dir_to_file (*i);

		XMLTree tree;

		if (!tree.read (file.c_str())) {
			continue;
		}

		TemplateInfo rti;

		rti.name = basename_nosuffix (*i);
		rti.path = *i;

		template_names.push_back (rti);
	}
}

void
find_route_templates (vector<TemplateInfo>& template_names)
{
	vector<string> templates;

	find_files_matching_filter (templates, route_template_search_path(), route_template_filter, 0, false, true);

	if (templates.empty()) {
		return;
	}

	for (vector<string>::iterator i = templates.begin(); i != templates.end(); ++i) {
		string fullpath = *i;

		XMLTree tree;

		if (!tree.read (fullpath.c_str())) {
			continue;
		}

		XMLNode* root = tree.root();

		TemplateInfo rti;

		rti.name = IO::name_from_state (*root->children().front());
		rti.path = fullpath;

		template_names.push_back (rti);
	}
}

}
