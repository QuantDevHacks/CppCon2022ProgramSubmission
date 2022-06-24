/*
	Copyright 2022 Daniel Hanson

	Redistribution and use in source and binary forms, 
	with or without modification, are permitted provided that 
	the following conditions are met:

	1. Redistributions of source code must retain the above 
	   copyright notice, this list of conditions and the following 
	   disclaimer.

	2. Redistributions in binary form must reproduce the above 
	   copyright notice, this list of conditions and the following 
	   disclaimer in the documentation and/or other materials 
	   provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its 
	   contributors  may be used to endorse or promote products 
	   derived from this software without specific prior written 
	   permission.

	 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
	 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
	 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
	 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
	 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
	 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
	 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
	 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
	 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
	 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
	 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 	 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
	 OF SUCH DAMAGE.

*/

export module ChronoDate;
import <chrono>;
namespace date = std::chrono;
using Date = std::chrono::year_month_day;

import <compare>;
import <exception>;
import <iostream>;
import <optional>;


export class ChronoDate
{
public:
	ChronoDate(int year, unsigned month, unsigned day);
	ChronoDate(const Date& ymd);
	ChronoDate();

	// Accessors:
//	Date operator()() const { return date_; };
	int year() const;
	unsigned month() const;
	unsigned day() const;
	int serial_date() const;
	date::year_month_day ymd() const;

	// Modifying methods and operators:
	ChronoDate& add_years(int rhs_years);
	ChronoDate& add_months(int rhs_months);
	ChronoDate& add_days(int rhs_days);

	unsigned operator - (const ChronoDate& rhs) const;

	ChronoDate& operator ++ ();
	ChronoDate& operator -- ();

	//ChronoDate& operator += (int days);
	//ChronoDate& operator -= (int days);

	// Comparison operators
	bool operator == (const ChronoDate& rhs) const;
	std::strong_ordering operator <=> (const ChronoDate& rhs) const;

	// Check state:
	bool end_of_month() const;
	int days_in_month() const;
	bool leap_year() const;
	std::optional<date::weekday> weekend() const;


	// friend operator so that we can output date details with cout
	friend std::ostream& operator << (std::ostream& os, const ChronoDate& rhs);

private:
	date::year_month_day date_;
};

ChronoDate::ChronoDate(int year, unsigned month, unsigned day) : 
	date_{ date::year{year} / date::month{month} / date::day{day} }
{
	if(!date_.ok())		// std::chrono member function to check if valid date
	{
		std::exception e("ChronoDate constructor: Invalid date.");
		throw e;		
	}
	
	// This does nothing; see eom_start_no_mod_fwd() in non-member tests
	//if(end_of_month())
	//{
	//	// Implicit conversion to year_month_day type:
	//	date_ = date_.year() / date_.month() / date::last;
	//}
}

ChronoDate::ChronoDate(const Date& ymd):date_{ymd} 
{
	if (!date_.ok())		// std::chrono member function to check if valid date
	{
		std::exception e("ChronoDate constructor: Invalid year_month_day input.");
		throw e;
	}
}

ChronoDate::ChronoDate():date_{date::year(1970), date::month{1}, date::day{1} } {}

// chrono::year can be cast to int
// See https://en.cppreference.com/w/cpp/chrono/year/operator_int
int ChronoDate::year() const
{
	return static_cast<int>(date_.year());
}

// chrono::month can be cast to unsigned
// Note this is different from chrono::year (int)
// See https://en.cppreference.com/w/cpp/chrono/month/operator_unsigned
unsigned ChronoDate::month() const
{
	return static_cast<unsigned>(date_.month());
}

// chrono::day can be cast to unsigned
// Note this is different from chrono::year (int)
// See https://en.cppreference.com/w/cpp/chrono/day/operator_unsigned
unsigned ChronoDate::day() const
{
	return static_cast<unsigned>(date_.day());
}

int ChronoDate::serial_date() const
{
	return date::sys_days(date_).time_since_epoch().count();
}

std::chrono::year_month_day ChronoDate::ymd() const
{
	return date_;
}

ChronoDate& ChronoDate::add_years(int rhs_years)
{	
	bool prev_is_eom = end_of_month();
	date_ += date::years(rhs_years);

	if (month() == 2 && prev_is_eom)	// February = 2
	{
		// Implicit conversion to year_month_day type:
		date_ = date_.year() / date_.month() / date::last;
	}

	if (!date_.ok())
	{
		std::exception e("ChronoDate::ChronoDate::addYears(.): resulting date invalid.");
		throw e;
	}

	return *this;
}

ChronoDate& ChronoDate::add_months(int rhs_months)
{
	if (!end_of_month())
	{	
		// This is going to get convoluted:		
		if (!(this -> day() == 30 || this->day() == 29))	
		{
			date_ += date::months(rhs_months);
		}
		else
		{
			date::year_month_day_last temp{ date_.year() / date_.month() / date::last };
			temp += date::months{ rhs_months };
			if (temp.month() == date::month{ 2 })
			{
				date_ = temp;
			}
			else
			{
				date_ += date::months{ rhs_months };
			}
		}
	}
	else
	{
		date::year_month_day_last eom_last{ date_.year() / date_.month() / date::last };
		eom_last += date::months(rhs_months);
		date_ = eom_last;		// Implicit conversion to date::year_month_day type
	}

	if (!date_.ok())
	{
		std::exception e("ChronoDate::ChronoDate::addMonths(.): resulting date invalid.");
		throw e;
	}

	return *this;
}

ChronoDate& ChronoDate::add_days(int rhs_days)
{
	// Note that adding days is handled differently, per Howard Hinnant.
	// See https://stackoverflow.com/questions/62734974/how-do-i-add-a-number-of-days-to-a-date-in-c20-chrono
	date_ = date::sys_days(date_) + date::days(rhs_days);

	if (!date_.ok())
	{
		std::exception e("ChronoDate::ChronoDate::addDays(.): resulting date out of range.");
		throw e;
	}

	return *this;
}

unsigned ChronoDate::operator - (const ChronoDate& rhs) const
{
	return (date::sys_days(date_).time_since_epoch()
		- date::sys_days(rhs.date_).time_since_epoch()).count();
}

ChronoDate& ChronoDate::operator ++ ()
{
	return add_days(1);
}

ChronoDate& ChronoDate::operator -- ()
{
	return add_days(-1);
}

bool ChronoDate::operator == (const ChronoDate& rhs) const
{
	return date_ == rhs.date_;
}

std::strong_ordering ChronoDate::operator <=> (const ChronoDate& rhs) const
{
	return date_ <=> rhs.date_;
}


bool ChronoDate::end_of_month() const
{
	return date_ == date_.year() / date_.month() / date::last;
}


int ChronoDate::days_in_month() const
{
	// See https://stackoverflow.com/questions/59418514/using-c20-chrono-how-to-compute-various-facts-about-a-date
	// Fact 5: Number of days in month (section)
	auto num_days_in_mth = date::sys_days{ date_.year() / date_.month() / date::last }
	- date::sys_days{ date_.year() / date_.month() / 1 } + date::days{ 1 };

	return num_days_in_mth.count();
}

bool ChronoDate::leap_year() const
{
	return date_.year().is_leap();
}

std::optional<date::weekday> ChronoDate::weekend() const
{
	date::weekday day_of_week{ date::sys_days(date_) };
	if (!(day_of_week == date::Saturday || day_of_week == date::Sunday))
	{
		return std::nullopt;
	}
	else
	{
		return day_of_week;			// Saturday or Sunday
	}
}

// This is a 'friend' of the ChronoDate class; also
// requires separate export keyword even though the 
// class declaration is export(ed).
export std::ostream& operator << (std::ostream& os, const ChronoDate& rhs)
{
	os << rhs.ymd();
	return os;
}
