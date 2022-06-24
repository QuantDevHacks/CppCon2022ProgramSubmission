module;
#include <cmath>

export module Bond;

import <utility>;		// std::move
import <vector>;		// To store calculated discount factors
using std::vector;

import ChronoDate;
import DayCounts;
import TermStructure;
import Schedule;

//	Schedule(const ChronoDate& val_date, const ChronoDate& first_reg_date, 
//		const ChronoDate& last_date, int tenor);

export class Bond
{
public:
	Bond(const ChronoDate& sett_date, const ChronoDate& first_cpn_date,
		const ChronoDate& mat_date, int tenor, double coupon_rate, /*const DayCount& day_count,*/
		double face_value, TermStructure&& term_struct);

	double operator()() const;		// Returns calculated price

	// Test accessors
	std::vector<double> disc_factors() const
	{
		return disc_fctrs_;
	}


private:
	ChronoDate sett_date_, first_cpn_date_, mat_date_;		// Change to move/rvalue input later 
	int tenor_;
	double coupon_rate_, face_value_; 
//	const DayCount& day_count_;		// Change to unique_ptr later	(contained in TermStructure class)
	TermStructure term_struct_;
	double yield_{ 0.0 };			// To be determined from the term structure
//	double df_{ 0.0 };				// Discount factor -- to be calculated (non-const -- put in vector)
	vector<double> disc_fctrs_;
	Schedule sched_;
	double price_{ 0.0 };			// To be calculated

	// Private functions:
	void calculate_price_();
	void retrieve_yield_();
	void calculate_disc_factors_();
};

Bond::Bond(const ChronoDate& sett_date, const ChronoDate& first_cpn_date,
	const ChronoDate& mat_date, int tenor, double coupon_rate, double face_value, /*const DayCount& day_count,*/
	TermStructure&& term_struct):
	sett_date_{ sett_date }, first_cpn_date_{ first_cpn_date }, mat_date_{ mat_date },
	tenor_{ tenor }, coupon_rate_{ coupon_rate }, face_value_{ face_value }, /*day_count_{ day_count }, */
	term_struct_{ std::move(term_struct) }, sched_{ sett_date, first_cpn_date, mat_date, tenor }
{
	calculate_price_();
}

double Bond::operator()() const
{

	return price_;
}

void Bond::calculate_price_()
{
	retrieve_yield_();
	calculate_disc_factors_();

	double cpn_amount = (face_value_ * coupon_rate_ * tenor_) / 12.0;		// 12 months in year
	double pv_cpn_pmts{ 0.0 };
	for (double df : disc_fctrs_)
	{
		pv_cpn_pmts += df * cpn_amount;
	}

	price_ = disc_fctrs_.back() * face_value_ + pv_cpn_pmts;
}

void Bond::retrieve_yield_()
{
	yield_ = term_struct_.forward_rate(first_cpn_date_, mat_date_);
}

void Bond::calculate_disc_factors_()
{
	// 	double disc_factor(const ChronoDate& date1, const ChronoDate& date2) const;
//	disc_fctrs_.emplace_back(term_struct_.disc_factor(sett_date_, first_cpn_date_));

	for (unsigned i = 1; i < sched_().size(); ++i)
	{
//		disc_fctrs_.emplace_back(term_struct_.disc_factor(sched_().at(i - 1), sched_().at(i)));
		disc_fctrs_.emplace_back(term_struct_.disc_factor(sched_().at(0), sched_().at(i)));
	}
}