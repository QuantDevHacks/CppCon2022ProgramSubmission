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

export module DayCounts;

import ChronoDate;

export
{
	class DayCount
	{
	public:
		// The operator should just call the member function year_fraction(.)
		virtual double operator() (const ChronoDate& date1, const ChronoDate& date2) const = 0;
		virtual double year_fraction(const ChronoDate& date1, const ChronoDate& date2) const = 0;
		virtual ~DayCount() = default;
	};

	class Thirty360 : public DayCount
	{
	public:
		double operator() (const ChronoDate& date1, const ChronoDate& date2) const override;
		double year_fraction(const ChronoDate& date1, const ChronoDate& date2) const override;

	private:
		// Maybe we should make this public and virtual on all derived classes (?)
		unsigned dateDiff_(const ChronoDate& date1, const ChronoDate& date2) const;
	};

	class Act365 : public DayCount
	{
	public:
		double operator() (const ChronoDate& date1, const ChronoDate& date2) const override;
		double year_fraction(const ChronoDate& date1, const ChronoDate& date2) const override;
	};

	class Act360 : public DayCount
	{
	public:
		double operator() (const ChronoDate& date1, const ChronoDate& date2) const override;
		double year_fraction(const ChronoDate& date1, const ChronoDate& date2) const override;
	};

	class Thirty360Eur : public DayCount
	{
	public:
		double operator() (const ChronoDate& date1, const ChronoDate& date2) const override;
		double year_fraction(const ChronoDate& date1, const ChronoDate& date2) const override;

	private:
		// Maybe we should make this public and virtual on all derived classes (?)
		unsigned dateDiff_(const ChronoDate& date1, const ChronoDate& date2) const;
	};
}

// *** Class Act360 ***
double Act360::operator()(const ChronoDate& date1, const ChronoDate& date2) const
{
	return year_fraction(date1, date2);
}

double Act360::year_fraction(const ChronoDate& date1, const ChronoDate& date2) const
{
	return (date2 - date1) / 360.0;
}

// *** Class Act365 ***
double Act365::operator()(const ChronoDate& date1, const ChronoDate& date2) const
{
	return year_fraction(date1, date2);
}

double Act365::year_fraction(const ChronoDate& date1, const ChronoDate& date2) const
{
	return (date2 - date1) / 365.0;
}

// *** Class Thirty360 ***
	// This is the **ISDA** 30/360  daycount.
	// See https://www.iso20022.org/15022/uhb/mt565-16-field-22f.htm and
	// http://kalotay.com/SupportingDocumentation/BondOAS/daycounts.html (30/360 ISDA)
double Thirty360::operator()(const ChronoDate& date1, const ChronoDate& date2) const
{
	return year_fraction(date1, date2);
}

double Thirty360::year_fraction(const ChronoDate& date1, const ChronoDate& date2) const
{
	return static_cast<double>(dateDiff_(date1, date2)) / 360.0;
}


double Thirty360Eur::operator()(const ChronoDate& date1, const ChronoDate& date2) const
{
	return year_fraction(date1, date2);
}

double Thirty360Eur::year_fraction(const ChronoDate& date1, const ChronoDate& date2) const
{

	return static_cast<double>(dateDiff_(date1, date2)) / 360.0;
}

unsigned Thirty360Eur::dateDiff_(const ChronoDate& date1, const ChronoDate& date2) const
{
	unsigned d1, d2;
	d1 = date1.day();
	d2 = date2.day();

	auto f = [](unsigned& d) {
		if (d == 31)
		{
			d = 30;
		}
	};

	f(d1);
	f(d2);

	return 360 * (date2.year() - date1.year()) + 30 * (date2.month() - date1.month()) + d2 - d1;
}


unsigned Thirty360::dateDiff_(const ChronoDate& date1, const ChronoDate& date2) const
{
	unsigned d1, d2;
	d1 = date1.day();
	d2 = date2.day();

	if (d1 == 31) d1 = 30;

	// Actual rule is conditioned on d1 being 30 *or* 31,
	// but the case of 31 was already adjusted in the previous command
	if ((d2 == 31) && (d1 == 30)) d2 = 30;

	return 360 * (date2.year() - date1.year()) + 30 * (date2.month() - date1.month()) + d2 - d1;
}
