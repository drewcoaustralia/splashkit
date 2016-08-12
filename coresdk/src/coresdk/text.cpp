//
//  text.cpp
//  splashkit
//
//  Created by Andrew Cain on 25/07/2016.
//  Copyright © 2016 Andrew Cain. All rights reserved.
//

#include "text.h"
#include "backend_types.h"
#include "resources.h"
#include "utility_functions.h"

#include "text_driver.h"
#include "graphics_driver.h"

#include <map>

static map<string, font> _fonts;

bool has_font(string name)
{
    return _fonts.count(name) > 0;
}

bool font_has_size(string name, int font_size)
{
    bool font_exists = has_font(name);
    bool font_size_exists = _fonts[name]->_data.count(font_size) > 0;

    return font_exists and font_size_exists;
}

font font_named(string name)
{
    if (has_font(name))
        return _fonts[name];
    else
        return nullptr;
}

void free_font(font fnt)
{
    if ( VALID_PTR(fnt, FONT_PTR) )
    {
        _fonts.erase(fnt->name);
        sk_close_font(fnt);
        fnt->id = NONE_PTR;  // ensure future use of this pointer will fail...
        delete(fnt);
    }
    else
    {
        raise_warning("Delete font called without valid font");
    }
}

void free_all_fonts()
{
    string name;
    
    size_t sz = _fonts.size();
    
    for(size_t i = 0; i < sz; i++)
    {
        font fnt = _fonts.begin()->second;
        if (VALID_PTR(fnt, FONT_PTR))
        {
            free_font(fnt);
        }
        else
        {
            raise_warning("Fonts contained an invalid pointer");
            _fonts.erase(_fonts.begin());
        }
    }
}

void set_font_style(font fnt, int font_size, font_style style)
{
    if (!VALID_PTR(fnt, FONT_PTR))
    {
        raise_warning("Attempting to set style on invalid font.");
        return;
    }

    sk_set_font_style(fnt, font_size, style);
}

font_style get_font_style(font fnt, int font_size)
{
    if (!VALID_PTR(fnt, FONT_PTR)) {
        raise_warning("Attempting to get font style on invalid font.");
        return NORMAL_FONT; // Add NONE to font_style enum?
    }

    // Should the backend not just return a font_style instead of an int?
    return static_cast<font_style>(sk_get_font_style(fnt, font_size));
}

font load_font(string name, string filename)
{
    if (has_font(name)) return font_named(name);

    string file_path = path_to_resource(filename, FONT_RESOURCE);
    
    if ( ! file_exists(file_path) )
    {
        file_path = path_to_resource(filename + ".ttf", FONT_RESOURCE);
    
        if ( ! file_exists(file_path) )
        {
            raise_warning(cat({ "Unable to locate file for ", name, " (", file_path, ")"}));
            return nullptr;
        }
    }

    font result = sk_load_font(file_path.c_str(), 64);
    result->name = name; // Need to clean this up, name is set to filename in sk_load_font
    
    if (!sk_contains_valid_font(result))
    {
        delete result;
        result = nullptr;
        raise_warning("LoadFont failed: " + name + " (" + file_path + ")");
    } else {
        _fonts.insert(std::make_pair(name, result));
    }

    return result;
}

void font_load_size(string name, int font_size)
{
    if (_fonts.count(name) > 0) {
        font fnt = _fonts[name];
        sk_add_font_size(fnt, font_size);
    } else {
        raise_warning("font_load_size failed: font named \"" + name + "\" does not exist.");
    }
}

void _print_strings(void *dest, font fnt, int font_size, string str, rectangle rc, color fg_clr, color bg_clr, font_alignment flags)
{
    if (bg_clr.a > 0)
    {
        sk_fill_aa_rect(to_surface_ptr(dest), bg_clr, rc.x, rc.y, rc.width, rc.height);
    }
    
    sk_draw_text(to_surface_ptr(dest), fnt, font_size, rc.x, rc.y, str.c_str(), fg_clr);
}

void draw_text(string text, color clr, font fnt, int font_size, float x, float y, drawing_options opts)
{
    rectangle rect;
    if ( ! VALID_PTR(fnt, FONT_PTR) )
    {
        raise_warning("Error attempting to draw text with invalid font.");
        return;
    }
    
    if (text.length() < 1) return;

    xy_from_opts(opts, x, y);
    rect.x = x;
    rect.y = y;

    rect.width = -1;
    rect.height = -1;
    
    _print_strings(opts.dest, fnt, font_size, text, rect, clr, COLOR_TRANSPARENT, ALIGN_LEFT);
}

void draw_text(string text, color clr, font fnt, int font_size, float x, float y)
{
    draw_text(text, clr, fnt, font_size, x, y, option_defaults());
}

void draw_text(string text, color clr, float x, float y, drawing_options opts)
{
    xy_from_opts(opts, x, y);
    sk_draw_text(to_surface_ptr(opts.dest), nullptr, x, y, text.c_str(), clr);
}