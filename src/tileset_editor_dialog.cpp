/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef NO_EDITOR
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <iostream>

#include "border_widget.hpp"
#include "button.hpp"
#include "editor.hpp"
#include "foreach.hpp"
#include "grid_widget.hpp"
#include "input.hpp"
#include "label.hpp"
#include "preview_tileset_widget.hpp"
#include "raster.hpp"
#include "tileset_editor_dialog.hpp"

namespace editor_dialogs
{

namespace {

std::set<tileset_editor_dialog*>& all_tileset_editor_dialogs() {
	static std::set<tileset_editor_dialog*> all;
	return all;
}
}

void tileset_editor_dialog::global_tile_update()
{
	foreach(tileset_editor_dialog* d, all_tileset_editor_dialogs()) {
		d->init();
	}
}

tileset_editor_dialog::tileset_editor_dialog(editor& e)
  : dialog(graphics::screen_width() - EDITOR_SIDEBAR_WIDTH, 160, EDITOR_SIDEBAR_WIDTH, 440), editor_(e), first_index_(-1)
{
	all_tileset_editor_dialogs().insert(this);

	set_clear_bg_amount(255);
	if(editor_.all_tilesets().empty() == false) {
		category_ = editor_.all_tilesets().front().category;
	}

	init();
}

tileset_editor_dialog::~tileset_editor_dialog()
{
	all_tileset_editor_dialogs().erase(this);
}

void tileset_editor_dialog::init()
{
	clear();
	using namespace gui;
	set_padding(20);

	assert(editor_.get_tileset() >= 0 && editor_.get_tileset() < editor_.all_tilesets().size());

	button* category_button = new button(widget_ptr(new label(category_, graphics::color_white())), boost::bind(&tileset_editor_dialog::show_category_menu, this));
	add_widget(widget_ptr(category_button), 10, 10);

	grid_ptr grid(new gui::grid(3));
	int index = 0, first_index = -1;
	first_index_ = -1;
	foreach(const editor::tileset& t, editor_.all_tilesets()) {
		if(t.category == category_) {
			if(first_index_ == -1) {
				first_index_ = index;
			}
			preview_tileset_widget* preview = new preview_tileset_widget(*t.preview());
			preview->set_dim(40, 40);
			button_ptr tileset_button(new button(widget_ptr(preview), boost::bind(&tileset_editor_dialog::set_tileset, this, index)));
			tileset_button->set_dim(44, 44);
			grid->add_col(gui::widget_ptr(new gui::border_widget(tileset_button, index == editor_.get_tileset() ? graphics::color(255,255,255,255) : graphics::color(0,0,0,0))));
		}

		++index;
	}

	grid->finish_row();
	add_widget(grid);
}

void tileset_editor_dialog::select_category(const std::string& category)
{
	category_ = category;
	init();

	if(first_index_ != -1) {
		set_tileset(first_index_);
	}
}

void tileset_editor_dialog::close_context_menu(int index)
{
	remove_widget(context_menu_);
	context_menu_.reset();
}

void tileset_editor_dialog::show_category_menu()
{
	using namespace gui;
	gui::grid* grid = new gui::grid(2);
	grid->swallow_clicks();
	grid->set_show_background(true);
	grid->set_hpad(10);
	grid->allow_selection();
	grid->register_selection_callback(boost::bind(&tileset_editor_dialog::close_context_menu, this, _1));

	std::set<std::string> categories;
	foreach(const editor::tileset& t, editor_.all_tilesets()) {
		if(categories.count(t.category)) {
			continue;
		}

		categories.insert(t.category);

		preview_tileset_widget* preview = new preview_tileset_widget(*t.preview());
		preview->set_dim(48, 48);
		grid->add_col(widget_ptr(preview))
		     .add_col(widget_ptr(new label(t.category, graphics::color_white())));
		grid->register_row_selection_callback(boost::bind(&tileset_editor_dialog::select_category, this, t.category));
	}

	int mousex, mousey;
	input::sdl_get_mouse_state(&mousex, &mousey);
	if(mousex + grid->width() > graphics::screen_width()) {
		mousex = graphics::screen_width() - grid->width();
	}

	if(mousey + grid->height() > graphics::screen_height()) {
		mousey = graphics::screen_height() - grid->height();
	}

	mousex -= x();
	mousey -= y();

	remove_widget(context_menu_);
	context_menu_.reset(grid);
	add_widget(context_menu_, mousex, mousey);
}

void tileset_editor_dialog::set_tileset(int index)
{
	if(editor_.get_tileset() != index) {
		editor_.set_tileset(index);
		init();
	}
}

bool tileset_editor_dialog::handle_event(const SDL_Event& event, bool claimed)
{
	if(!claimed) {
		if(context_menu_) {
			gui::widget_ptr ptr = context_menu_;
			SDL_Event ev = event;
			normalize_event(&ev);
			return ptr->process_event(ev, claimed);
		}

		switch(event.type) {
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_COMMA) {
				editor_.set_tileset(editor_.get_tileset()-1);
				while(editor_.all_tilesets()[editor_.get_tileset()].category != category_) {
					editor_.set_tileset(editor_.get_tileset()-1);
				}
				set_tileset(editor_.get_tileset());
				claimed = true;
			} else if(event.key.keysym.sym == SDLK_PERIOD) {
				editor_.set_tileset(editor_.get_tileset()+1);
				while(editor_.all_tilesets()[editor_.get_tileset()].category != category_) {
					editor_.set_tileset(editor_.get_tileset()+1);
				}
				set_tileset(editor_.get_tileset());
				claimed = true;
			}
			break;
		}
	}

	return dialog::handle_event(event, claimed);
}
}
#endif // !NO_EDITOR
