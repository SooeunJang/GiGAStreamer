/*
 * event.hpp
 *
 *  Created on: 2018. 6. 22.
 *      Author: jangsueun
 */
#pragma once

#include<string>
#include<vector>

namespace cfx
{
	class Event
	{
	public:
		Event() : id(0), data(NULL) {}
		~Event() {}
		void set_event(int _id, std::string _data)
		{
			id = _id;
			data = _data;
			std::cout<<id<<":"<<data<<std::endl;
		}
		int get_id() { return id; }
		std::string get_data()
		{
			return data;
		}
//	private:
		int id;
		std::string data;
	};
	typedef std::vector<Event*> EventQueue;

}
