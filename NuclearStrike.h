#pragma once
#ifndef _NUCLEAR_STRIKE_H_
#define _NUCLEAR_STRIKE_H_

#include "Action.h"
#include "Vect.h"

// Orders vehicle with <launcher_id> to deliver Nuclear Strike at <target>
class NuclearStrike : public Action {
 public:
	NuclearStrike(const Vect& target, const long long launcher_id);	
	void Execute(model::Move& move) const override;
	std::string Name() const override;

 private:
	Vect target_;
	long long launcher_id_;
};

#endif