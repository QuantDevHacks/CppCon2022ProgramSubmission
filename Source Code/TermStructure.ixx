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

module;
#include <cmath>

export module TermStructure;

import <vector>;
using std::vector;
import <utility>;		// std::move
import <exception>;
import <memory>;

import ChronoDate;
import DayCounts;
import Interpolation;

export class TermStructure
{
public:
	TermStructure(std::vector<ChronoDate>&& dates, std::vector<double>&& rates, 
		std::unique_ptr<DayCount> day_count);
	TermStructure(std::vector<ChronoDate>&& dates, std::vector<double>&& rates);
	double forward_rate(const ChronoDate& date1, const ChronoDate& date2) const;
	double disc_factor(const ChronoDate& date1, const ChronoDate& date2) const;
	double year_fraction(const ChronoDate& date1, const ChronoDate& date2) const;

private:
	LinearInterpolation lin_interp_;
	bool check_dates_(const ChronoDate& date1, const ChronoDate& date2) const;
	//Act360 day_count_;
	std::unique_ptr<DayCount> day_count_;
};

TermStructure::TermStructure(std::vector<ChronoDate>&& dates, std::vector<double>&& rates, 
	std::unique_ptr<DayCount> day_count):
	lin_interp_{ std::move(dates), std::move(rates) }, day_count_{ std::move(day_count) } {}

TermStructure::TermStructure(std::vector<ChronoDate>&& dates, std::vector<double>&& rates) :
	lin_interp_{ std::move(dates), std::move(rates) }, day_count_{ std::make_unique<Act360>() } {}

double TermStructure::forward_rate(const ChronoDate& date1, const ChronoDate& date2) const
{
	return -std::log(disc_factor(date1, date2)) / (*day_count_)(date1, date2);
}

double TermStructure::disc_factor(const ChronoDate& date1, const ChronoDate& date2) const
{
	if (check_dates_(date1, date2))
	{
		double rate1 = lin_interp_(date1);
		double rate2 = lin_interp_(date2);
		
		double disc_fctr1 = std::exp(-rate1 * (*day_count_)(lin_interp_.value_date(), date1));
		double disc_fctr2 = std::exp(-rate2 * (*day_count_)(lin_interp_.value_date(), date2));
		return disc_fctr2 / disc_fctr1;
	}
	else
	{
		std::exception e("TermStructure::disc_factor: Improper date ordering");
		throw e;
	}
}

double TermStructure::year_fraction(const ChronoDate& date1, const ChronoDate& date2) const
{
	return (*day_count_)(date1, date2);
}

bool TermStructure::check_dates_(const ChronoDate& date1, const ChronoDate& date2) const
{
	if (date2 < date1)
	{
		return false;
	}
	else if (date1 < lin_interp_.value_date() || date1 > lin_interp_.final_date())
	{
		return false;
	}
	else if (date2 < lin_interp_.value_date() || date2 > lin_interp_.final_date())
	{
		return false;
	}
	else
	{
		return true;
	}
}