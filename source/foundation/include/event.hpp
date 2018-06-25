/*
 * event.hpp
 *
 *  Created on: 2018. 6. 22.
 *      Author: jangsueun
 */
#pragma once

#include<string>
#include<vector>
#include "cpprest/json.h"
using namespace web;

namespace cfx
{
	typedef struct _Event
	{
		int id;
		json::value data;
		void set_event(int _id, json::value _data)
		{
			id = _id;
			data = _data;
		}
		int get_id() { return id; }
		json::value get_data()
		{
			return data;
		}
	}Event;

	typedef std::vector<Event> EventQueue;

}
