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
#include <cmath>		// std::lerp

export module Interpolation;
import <utility>;		// std::move
import <algorithm>;		// std::lower_bound
import <iterator>;		// std::distance
import <vector>;
using std::vector;

import ChronoDate;

export class LinearInterpolation
{
public:
	LinearInterpolation(std::vector<ChronoDate>&& dates, std::vector<double>&& rates):
		dates_{std::move(dates)}, rates_{std::move(rates)}
	{
		// In practice, should test if dates are sorted or not.	
	}

	double operator()(const ChronoDate& rhs) const
	{
		if (rhs <= dates_.front())
		{
			auto check{ dates_.front() };
			return rates_.front();
		}
		else if (rhs >= dates_.back())
		{
			auto check{ dates_.back() };
			return rates_.back();
		}
		else
		{
			// iter is actually an upper bound
			// Definitely counterintuitive...
			auto iter = std::lower_bound(dates_.cbegin(), dates_.cend(), rhs);
 			auto up_idx = std::distance(dates_.begin(), iter);
			if (rhs == *iter)
			{
				return rates_.at(up_idx);
			}
			else
			{
				// Run the interpolation...
				auto down_idx = up_idx - 1;
				auto t = (static_cast<double>(rhs - dates_.at(down_idx))) 
					/ (static_cast<double>(dates_.at(up_idx) - dates_.at(down_idx)));
				return std::lerp(rates_.at(down_idx), rates_.at(up_idx), t);				
			}
		}	
	}

	ChronoDate value_date() const
	{
		return dates_.front();
	}

	ChronoDate final_date() const
	{
		return dates_.back();
	}

private:

	vector<ChronoDate> dates_;
	vector<double> rates_;

	double interp_rate_;		// Not sure this will be used...

};