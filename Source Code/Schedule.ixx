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

export module Schedule;

import <algorithm>;			// std::min
import <chrono>;
namespace date = std::chrono;
using Date = std::chrono::year_month_day;

import <vector>;
using std::vector;

// TEMPORARY (REMOVE!)
import <iostream>;
using std::cout;
using std::endl;

import ChronoDate;

export class Schedule
{
public:
	Schedule(const ChronoDate& val_date, const ChronoDate& first_reg_date, 
		const ChronoDate& last_date, int tenor);

	std::vector<ChronoDate> operator()() const;

private:	
	ChronoDate val_date_, first_reg_date_, last_date_;
	int tenor_;
	std::vector<ChronoDate> schedule_;
	void generate_schedule_();
	ChronoDate mod_following_(const ChronoDate& check_date,
		const date::weekday& day_of_week);
};

Schedule::Schedule(const ChronoDate& val_date, const ChronoDate& first_reg_date,
	const ChronoDate& last_date, int tenor) : val_date_{ val_date }, 
	first_reg_date_{ first_reg_date }, last_date_{ last_date }, tenor_{tenor}
{
	// If production code, should also check if val_date <= first_reg_date <= last_date,
	// and tenor > 0.

	generate_schedule_();
}

std::vector<ChronoDate> Schedule::operator()() const
{
	return schedule_;
}

void Schedule::generate_schedule_()
{
	// Assume the first two dates are valid:
	schedule_.push_back(val_date_);
	schedule_.push_back(first_reg_date_);

	auto first_reg_day = first_reg_date_.day();

	auto next_date = first_reg_date_;
	while (next_date < last_date_)
	{
		ChronoDate test_last{ next_date.ymd().year() / next_date.ymd().month() / date::last };
		test_last.add_months(tenor_);
		next_date = ChronoDate{ test_last.year(), test_last.month(), std::min(test_last.day(), first_reg_day) };

		auto is_wknd = next_date.weekend();

		if (!is_wknd)
		{
			schedule_.push_back(next_date);
		}
		else
		{
			schedule_.emplace_back(mod_following_(next_date, *is_wknd));
		}

		//cout << schedule_.back() << endl;
	}
}

ChronoDate Schedule::mod_following_(const ChronoDate& check_date,
	const date::weekday& day_of_week)
{
	// This function is only called if the date is a weekend.
	auto copy_check = check_date;
	copy_check.add_months(tenor_);
	if (check_date.month() == copy_check.month())		// Does not advance to next month
	{
		if (day_of_week == date::Saturday)
		{
			auto adj = Date{ date::sys_days(check_date.ymd()) + date::days(2) };
			ChronoDate ret{ adj };
			return ret;
		}
		else // day_of_week == date::Sunday
		{
			auto adj = Date{ date::sys_days(check_date.ymd()) + date::days(1) };
			ChronoDate ret{ adj };
			return ret;
		}
	}
	else
	{
		if (day_of_week == date::Saturday)				// Does advance to next month
		{
			auto adj = Date{ date::sys_days(check_date.ymd()) - date::days(1) };
			ChronoDate ret{ adj };
			return ret;
		}
		else	// day_of_week == date::Sunday
		{
			auto adj = Date{ date::sys_days(check_date.ymd()) - date::days(2) };
			ChronoDate ret{ adj };
			return ret;
		}
	}
}