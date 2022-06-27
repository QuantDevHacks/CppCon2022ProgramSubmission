module;
#include <cmath>

export module Cashflows;

import <utility>;
import <memory>;
import <algorithm>;
import <ranges>;
import <numeric>;
import <vector>;
using std::vector;

import ChronoDate;
import Schedule;
import TermStructure;
import DayCounts;

// This will be moved elsewhere later -- put here 
// to test integer representation of an enum class.
export enum class PayRec
{
	Payer = 1,
	Receiver = -1
};

export class CashflowLeg
{
public:
	CashflowLeg(const Schedule& sched, TermStructure&& term_struct,
		double notional, double fixed_rate);	

	CashflowLeg(const Schedule& sched, TermStructure&& term_struct,
		double notional);

	double present_val() const;
	vector<double> operator()() const;

private:
	Schedule sched_;
	TermStructure term_struct_;
	double notional_;
	double fixed_rate_;
	vector<double> pv_cash_flows_;

	void generate_fixed_cash_flows_();
	void generate_float_cash_flows_();

};

CashflowLeg::CashflowLeg(const Schedule& sched, TermStructure&& term_struct,
	double notional, double fixed_rate) :sched_{ sched }, term_struct_{ std::move(term_struct) },
	notional_{ notional }, fixed_rate_{ fixed_rate }
{
	pv_cash_flows_.reserve(sched_().size());
	generate_fixed_cash_flows_();
}

CashflowLeg::CashflowLeg(const Schedule& sched, TermStructure&& term_struct,
	double notional) :sched_{ sched }, term_struct_{ std::move(term_struct) },
	notional_{ notional }
{
	pv_cash_flows_.reserve(sched_().size());
	generate_float_cash_flows_();
}

double CashflowLeg::present_val() const
{
	return std::accumulate(pv_cash_flows_.begin(), pv_cash_flows_.end(), 0.0);
}

void CashflowLeg::generate_fixed_cash_flows_()
{
	pv_cash_flows_.push_back(0.0);			// Value date
	pv_cash_flows_.push_back(0.0);			// First "reset" date
	for(unsigned i = 2; i < sched_().size(); ++i)
	{
		pv_cash_flows_.push_back(
			term_struct_.disc_factor(sched_().front(), sched_().at(i)) *
			term_struct_.year_fraction(sched_().at(i - 1), sched_().at(i)) *
			fixed_rate_
		);
	}

	std::transform(pv_cash_flows_.begin(), pv_cash_flows_.end(), pv_cash_flows_.begin(),
		[this](double x) {return this->notional_ * x; });
}

void CashflowLeg::generate_float_cash_flows_()
{
	pv_cash_flows_.push_back(0.0);			// Value date
	pv_cash_flows_.push_back(0.0);			// First "reset" date
	for (unsigned i = 2; i < sched_().size(); ++i)
	{
		auto test1 = term_struct_.disc_factor(sched_().front(), sched_().at(i));
		auto test2 = std::exp(term_struct_.forward_rate(sched_().front(), sched_().at(i)));
		pv_cash_flows_.push_back(
			term_struct_.disc_factor(sched_().front(), sched_().at(i)) *
			term_struct_.forward_cont_rate(sched_().at(i - 1), sched_().at(i))
		);
	}

	std::transform(pv_cash_flows_.begin(), pv_cash_flows_.end(), pv_cash_flows_.begin(),
		[this](double x) {return this->notional_ * x; });
}

vector<double> CashflowLeg::operator()() const
{
	return pv_cash_flows_;
}