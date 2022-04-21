// PinkRabbit
// 2022-04-22
// An efficient DFA class implement

#ifndef DFA_H__PINKRABBIT
#define DFA_H__PINKRABBIT

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>

#define RE(...) void(), std::fprintf(stderr, __VA_ARGS__), std::exit(23), void()
#define WARN(...) void(), std::fprintf(stderr, __VA_ARGS__), void()

std::string symbolize(int x) {
	if (x >= 0 && x < 128 && isgraph(x))
		return std::string(1, (char)x);
	return "\"" + std::to_string(x) + "\"";
}

class Alphabet {
	int siz;
	std::function<int(int)> func, inv;
public:
	Alphabet() : siz(0) {}
	Alphabet(int s, std::function<int(int)> fun, std::function<int(int)> invfun)
		: siz(s), func(fun), inv(invfun) {
		if (s <= 0)
			RE("[Error] Alphabet : Provided size (%d) is a non-positive integer.", siz);
	}
	Alphabet(const Alphabet &) = default;
	Alphabet(Alphabet &&) = default;
	Alphabet &operator = (const Alphabet &) = default;
	Alphabet &operator = (Alphabet &&) = default;
	void reset(int s, std::function<int(int)> fun, std::function<int(int)> invfun) {
		if (s <= 0)
			RE("[Error] Alphabet::reset : Provided size (%d) is a non-positive integer.", siz);
		siz = s, func = fun, inv = invfun;
	}
	void clear() {
		siz = 0;
		func = std::function<int(int)>();
		inv = std::function<int(int)>();
	}
	bool empty() const {
		return siz == 0;
	}
	int size() const {
		return siz;
	}
	int i2c(int id) const {
		if (id < 0 || id >= siz)
			RE("[Error] Alphabet::i2c : Invalid symbol id (%d) violates the range [0, %d].", id, siz - 1);
		return func(id);
	}
	int c2i(int c) const {
		int id = inv(c);
		if (id < 0 || id >= siz)
			RE("[Error] Alphabet::c2i : Invalid symbol \'%s\' is not in the alphabet.", symbolize(c).c_str());
		return id;
	}
	int operator () (int id) const {
		return i2c(id);
	}
	~Alphabet() = default;
};

class DeterministicFiniteAutomaton {
private:
	int n;
	Alphabet sig;
	std::vector<std::string> stateNames;
	std::vector<std::vector<int>> transitions;
	int countEmptyTransitions;
	bool isComplete;
	int startStateIndex;
	std::vector<unsigned char> acceptStates; // not to use std::vector<bool>
public:
	DeterministicFiniteAutomaton() : n(0), countEmptyTransitions(0), isComplete(false), startStateIndex(-1) {}
	DeterministicFiniteAutomaton(const Alphabet &alph) : n(0), sig(alph), countEmptyTransitions(0), isComplete(false), startStateIndex(-1) {}
	DeterministicFiniteAutomaton(const DeterministicFiniteAutomaton &) = default;
	DeterministicFiniteAutomaton(DeterministicFiniteAutomaton &&) = default;
	DeterministicFiniteAutomaton &operator = (const DeterministicFiniteAutomaton &) = default;
	DeterministicFiniteAutomaton &operator = (DeterministicFiniteAutomaton &&) = default;
	bool empty() const {
		return n == 0;
	}
	int size() const {
		return n;
	}
	bool complete() const {
		return isComplete;
	}
	const Alphabet &alphabet() const {
		return sig;
	}
	bool alphabetEmpty() const {
		return sig.empty();
	}
	int alphabetSize() const {
		return sig.size();
	}
	void clear() {
		n = 0;
		sig.clear();
		stateNames.clear();
		transitions.clear();
		countEmptyTransitions = 0;
		isComplete = false;
		startStateIndex = -1;
		acceptStates.clear();
	}
	void initTransitions() {
		if (n == 0 && sig.empty())
			return ; // OK. Initializing a completely empty automaton does nothing.
		if (n == 0)
			RE("[Error] DFA::initTransitions : Empty states but with an alphabet.");
		if (sig.empty())
			RE("[Error] DFA::initTransitions : Empty alphabet but with some states.");
		int m = sig.size();
		transitions.clear();
		transitions.resize(n);
		for (int i = 0; i < n; ++i)
			transitions[i].resize(m, -1);
		countEmptyTransitions = n * m;
		isComplete = false;
	}
	void setAlphabet(const Alphabet &alph) {
		if (alph.empty())
			RE("[Error] DFA::setAlphabet : Provided alphabet is empty.");
		if (!sig.empty())
			RE("[Error] DFA::setAlphabet : Alphabet already exists.");
		sig = alph;
		if (n != 0)
			initTransitions();
	}
	void resetAlphabet(const Alphabet &alph) {
		if (alph.empty())
			RE("[Error] DFA::resetAlphabet : Provided alphabet is empty.");
		if (sig.size() == alph.size()) {
			sig = alph;
			return ;
		}
		WARN("[Warning] DFA::resetAlphabet : The new alphabet\'s size (%d) didn\'t match the old one (%d).", alph.size(), sig.size());
	}
	void setSize(int siz) {
		if (n != 0)
			RE("[Error] DFA::setSize : Original DFA is not empty.");
		if (siz <= 0)
			RE("[Error] DFA::setSize : Provided size (%d) is a non-positive integer.", siz);
		n = siz;
		stateNames.resize(n);
		acceptStates.resize(n, 0u);
		if (!sig.empty())
			initTransitions();
	}
	class State {
		friend class DeterministicFiniteAutomaton;
	private:
		DeterministicFiniteAutomaton const *orig;
		int ind;
		bool vali;
		State(const DeterministicFiniteAutomaton &dfa) : orig(&dfa), ind(-1), vali(false) {} // invalid state constructor (private)
	public:
		State(const DeterministicFiniteAutomaton &dfa, int indnum) : orig(&dfa), ind(indnum), vali(true) {
			if (orig->empty())
				RE("[Error] DFA::State : Empty automaton.");
			if (ind < 0 || ind >= orig->size())
				RE("[Error] DFA::State : Invalid state internal index (%d) violates the range [0, %d].", ind, orig->size() - 1);
		}
		State(const State &) = default;
		State(State &&) = default;
		State &operator = (const State &) = default;
		State &operator = (State &&) = default;
		const DeterministicFiniteAutomaton &DFA() const {
			return *orig;
		}
		bool valid() const {
			return vali;
		}
		int index() const {
			if (!vali)
				return -1;
			return ind;
		}
		std::string name() const;
		State transition(int symbol) const;
		State transition(const std::vector<int> &symbols) const;
		State transition(const std::string &symbString) const;
		bool accept() const;
		~State() = default;
	};
	State getState(int indnum) const {
		if (n == 0)
			RE("[Error] DFA::getState : Empty automaton.");
		if (indnum < 0 || indnum >= n)
			RE("[Error] DFA::getState : Invalid state internal index (%d) violates the range [0, %d].", indnum, n - 1);
		return State(*this, indnum);
	}
	State operator [] (int indnum) const {
		if (n == 0)
			RE("[Error] DFA::operator [] : Empty automaton.");
		if (indnum < 0 || indnum >= n)
			RE("[Error] DFA::operator [] : Invalid state internal index (%d) violates the range [0, %d].", indnum, n - 1);
		return State(*this, indnum);
	}
	State invalid() const {
		if (n == 0)
			RE("[Error] DFA::invalid : Empty automaton.");
		return State(*this);
	}
	void setStartState(const State &state) {
		if (n == 0)
			RE("[Error] DFA::setStartState : Empty automaton.");
		if (this != &state.DFA())
			RE("[Error] DFA::setStartState : State\'s original automaton is not this one.");
		if (!state.valid())
			RE("[Error] DFA::setStartState : State is invalid.");
		int index = state.index();
		if (index < 0 || index >= n)
			RE("[Error] DFA::setStartState : Invalid state internal index (%d) violates the range [0, %d].", index, n - 1);
		startStateIndex = index;
	}
	State start() const {
		if (n == 0)
			RE("[Error] DFA::start : Empty automaton.");
		if (startStateIndex == -1)
			return invalid();
		return State(*this, startStateIndex);
	}
	void setStateName(const State &state, const std::string &name) {
		if (n == 0)
			RE("[Error] DFA::setStateName : Empty automaton.");
		if (this != &state.DFA())
			RE("[Error] DFA::setStateName : State\'s original automaton is not this one.");
		if (!state.valid())
			RE("[Error] DFA::setStateName : State is invalid.");
		int index = state.index();
		if (index < 0 || index >= n)
			RE("[Error] DFA::setStateName : Invalid state internal index (%d) violates the range [0, %d].", index, n - 1);
		stateNames[index] = name;
	}
	std::string getStateName(const State &state) const {
		if (n == 0)
			RE("[Error] DFA::getStateName : Empty automaton.");
		if (this != &state.DFA())
			RE("[Error] DFA::getStateName : State\'s original automaton is not this one.");
		if (!state.valid())
			return std::string("invalid");
		int index = state.index();
		if (index < 0 || index >= n)
			RE("[Error] DFA::getStateName : Invalid state internal index (%d) violates the range [0, %d].", index, n - 1);
		if (stateNames[index].empty())
			return "q_" + std::to_string(index);
		return stateNames[index];
	}
	void setTransition(const State &state1, int symbol, const State &state2) {
		if (n == 0)
			RE("[Error] DFA::setTransition : Empty automaton.");
		if (sig.empty())
			RE("[Error] DFA::setTransition : Empty alphabet");
		if (this != &state1.DFA())
			RE("[Error] DFA::setTransition : State1\'s original automaton is not this one.");
		if (this != &state2.DFA())
			RE("[Error] DFA::setTransition : State2\'s original automaton is not this one.");
		int symbId = sig.c2i(symbol);
		if (!state1.valid())
			RE("[Error] DFA::setTransition : State1 is invalid.");
		int index1 = state1.index();
		if (index1 < 0 || index1 >= n)
			RE("[Error] DFA::setTransition : Invalid state1 internal index (%d) violates the range [0, %d].", index1, n - 1);
		int &refIndex = transitions[index1][symbId];
		if (!state2.valid()) {
			if (refIndex != -1) {
				if (countEmptyTransitions == 0)
					isComplete = false;
				++countEmptyTransitions;
			}
			refIndex = -1;
			return ;
		}
		int index2 = state2.index();
		if (index2 < 0 || index2 >= n)
			RE("[Error] DFA::setTransition : Invalid state2 internal index (%d) violates the range [0, %d].", index2, n - 1);
		if (refIndex == -1) {
			--countEmptyTransitions;
			if (countEmptyTransitions == 0)
				isComplete = true;
		}
		refIndex = index2;
	}
	State transition(const State &state, int symbol) const {
		if (n == 0)
			RE("[Error] DFA::transition : Empty automaton.");
		if (sig.empty())
			RE("[Error] DFA::transition : Empty alphabet");
		if (this != &state.DFA())
			RE("[Error] DFA::transition : State\'s original automaton is not this one.");
		int symbId = sig.c2i(symbol);
		if (!state.valid())
			return invalid();
		int index1 = state.index();
		if (index1 < 0 || index1 >= n)
			RE("[Error] DFA::transition : Invalid state internal index (%d) violates the range [0, %d].", index1, n - 1);
		int index2 = transitions[index1][symbId];
		if (index2 == -1)
			return invalid();
		return getState(index2);
	}
	State transition(const State &state, const std::vector<int> &symbols) const {
		if (n == 0)
			RE("[Error] DFA::transition : Empty automaton.");
		if (sig.empty())
			RE("[Error] DFA::transition : Empty alphabet");
		if (this != &state.DFA())
			RE("[Error] DFA::transition : State\'s original automaton is not this one.");
		std::vector<int> symbIds(symbols.size());
		for (int i = 0; i < (int)symbols.size(); ++i)
			symbIds[i] = sig.c2i(symbols[i]);
		if (!state.valid())
			return invalid();
		int index1 = state.index();
		if (index1 < 0 || index1 >= n)
			RE("[Error] DFA::transition : Invalid state internal index (%d) violates the range [0, %d].", index1, n - 1);
		int index2 = index1;
		for (int symbId : symbIds) {
			index2 = transitions[index2][symbId];
			if (index2 == -1)
				return invalid();
		}
		return getState(index2);
	}
	State transition(const State &state, const std::string &symbString) const {
		std::vector<int> symbols(symbString.length());
		for (int i = 0; i < (int)symbString.length(); ++i)
			symbols[i] = symbString[i];
		return transition(state, symbols);
	}
	void clearAcceptStates(bool value = false) {
		if (n == 0)
			return ; // OK.
		std::fill(acceptStates.begin(), acceptStates.end(), (unsigned char)value);
	}
	void flipStateAcceptance(const State &state) {
		if (n == 0)
			RE("[Error] DFA::flipAnAcceptState : Empty automaton.");
		if (this != &state.DFA())
			RE("[Error] DFA::flipAnAcceptState : State\'s original automaton is not this one.");
		if (!state.valid())
			RE("[Error] DFA::flipAnAcceptState : State is invalid.");
		int index = state.index();
		if (index < 0 || index >= n)
			RE("[Error] DFA::flipAnAcceptState : Invalid state internal index (%d) violates the range [0, %d].", index, n - 1);
		acceptStates[index] = acceptStates[index] ^ 1u;
	}
	void resetAcceptStates(const std::vector<unsigned char> &acc) {
		if (n == 0)
			RE("[Error] DFA::resetAcceptStates(std::vector<unsigned char>) : Empty automaton.");
		if ((int)acc.size() != n)
			RE("[Error] DFA::resetAcceptStates(std::vector<unsigned char>) : std::vector size didn\'t match.");
		acceptStates = acc;
	}
	void resetAcceptStates(const std::vector<bool> &acc) {
		if (n == 0)
			RE("[Error] DFA::resetAcceptStates(std::vector<bool>) : Empty automaton.");
		if ((int)acc.size() != n)
			RE("[Error] DFA::resetAcceptStates(std::vector<bool>) : std::vector size didn\'t match.");
		for (int i = 0; i < n; ++i)
			acceptStates[i] = (unsigned char)(bool)acc[i];
	}
	void resetAcceptStates(const std::vector<int> &states) {
		if (n == 0)
			RE("[Error] DFA::resetAcceptStates(std::vector<int>) : Empty automaton.");
		clearAcceptStates();
		for (int ind : states)
			acceptStates[ind] = 1u;
	}
	bool isAcceptState(const State &state) const {
		if (n == 0)
			RE("[Error] DFA::isAcceptState : Empty automaton.");
		if (this != &state.DFA())
			RE("[Error] DFA::isAcceptState : State\'s original automaton is not this one.");
		if (!state.valid())
			RE("[Error] DFA::isAcceptState : State is invalid.");
		int index = state.index();
		if (index < 0 || index >= n)
			RE("[Error] DFA::isAcceptState : Invalid state internal index (%d) violates the range [0, %d].", index, n - 1);
		return (bool)acceptStates[index];
	}
private:
	// table filling algorithm with a queue to efficiently select pairs which distinguishable
	// O(n^2 m), with n = #nodes, m = |alpabet|
	// assuming DFA is complete
	// discarded, use equivalenceClassesHopcroft for a better performance
	// only for studying purpose
	std::vector<int> equivalenceClassesTableFilling() const {
		int m = sig.size();
		std::vector<std::vector<std::vector<int>>> invTr(n, std::vector<std::vector<int>>(m));
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < m; ++j)
				invTr[transitions[i][j]][j].push_back(i);
		std::vector<std::vector<unsigned char>> distinguishable(n, std::vector<unsigned char>(n, 0u));
		std::vector<std::pair<int, int>> que(n * (n - 1) / 2);
		int head = 0, tail = 0;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < i; ++j)
				if (acceptStates[j] != acceptStates[i])
					distinguishable[i][j] = 1u,
					que[tail++] = {i, j};
		while (head != tail) {
			std::pair<int, int> p = que[head++];
			int x = p.first, y = p.second;
			for (int j = 0; j < m; ++j) {
				for (int u : invTr[x][j]) {
					for (int v : invTr[y][j]) {
						int tu = u, tv = v;
						if (tu < tv)
							std::swap(tu, tv);
						if (distinguishable[tu][tv])
							continue;
						distinguishable[tu][tv] = 1u;
						que[tail++] = {tu, tv};
					}
				}
			}
		}
		que.clear();
		invTr.clear();
		int classCnt = 0;
		std::vector<int> ret(n, -1);
		for (int i = 0; i < n; ++i) {
			if (ret[i] != -1)
				continue;
			ret[i] = classCnt;
			for (int j = i + 1; j < n; ++j)
				if (!distinguishable[j][i])
					ret[j] = classCnt;
			++classCnt;
		}
		return ret;
	}
	// Hopcroft algorithm (1971) with a refined implementation
	// O(m n log n), with n = #nodes, m = |alpabet|
	// assuming DFA is complete
	std::vector<int> equivalenceClassesHopcroft() const {
		bool allAccept = true, allReject = true;
		for (int i = 0; i < n; ++i)
			if (acceptStates[i])
				allReject = false;
			else
				allAccept = false;
		if (allAccept || allReject)
			return std::vector<int>(n, 0);
		int m = sig.size();
		std::vector<std::vector<std::vector<int>>> invTr(n, std::vector<std::vector<int>>(m));
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < m; ++j)
				invTr[transitions[i][j]][j].push_back(i);
		std::vector<int> classBegin(n);
		std::vector<int> classSize(n);
		std::vector<int> classElems(n);
		std::vector<int> classSplitCnt(n, 0);
		std::vector<std::vector<int>> classSplitElems(n);
		std::vector<int> elemBelong(n);
		std::vector<int> elemPosition(n);
		int num = 0;
		std::vector<std::pair<int, int>> splitterStack(n * m);
		std::vector<std::vector<unsigned char>> inStack(n, std::vector<unsigned char>(m, 0u));
		int stackTop = 0;
		auto Split = [&](int id, const std::vector<int> &elems) -> int {
			int jd = num++;
			int siz = (int)elems.size();
			int pb = classBegin[id];
			for (int j = 0; j < siz; ++j, ++pb) {
				int a = elems[j];
				int pa = elemPosition[a];
				int b = classElems[pb];
				elemPosition[a] = pb;
				elemPosition[b] = pa;
				classElems[pb] = a;
				classElems[pa] = b;
				elemBelong[a] = jd;
			}
			classBegin[jd] = classBegin[id];
			classBegin[id] = pb;
			classSize[jd] = siz;
			classSize[id] -= siz;
			return jd;
		};
		classBegin[num] = 0;
		classSize[num] = n;
		for (int i = 0; i < classSize[num]; ++i) {
			classElems[classBegin[num] + i] = i;
			elemBelong[i] = num;
			elemPosition[i] = classBegin[num] + i;
		}
		++num;
		Split(0, [&]() -> std::vector<int> {
			std::vector<int> ret;
			for (int i = 0; i < n; ++i)
				if (acceptStates[i])
					ret.push_back(i);
			return ret;
		}());
		for (int j = 0; j < m; ++j) {
			int id = classSize[0] < classSize[1] ? 0 : 1;
			splitterStack[stackTop++] = {id, j};
			inStack[id][j] = 1u;
		}
		while (stackTop) {
			std::pair<int, int> p = splitterStack[--stackTop];
			int Aid = p.first, Aj = p.second;
			inStack[Aid][Aj] = 0u;
			std::vector<int> candidates;
			for (int i = 0; i < classSize[Aid]; ++i) {
				int elem = classElems[classBegin[Aid] + i];
				for (int x : invTr[elem][Aj]) {
					int bel = elemBelong[x];
					if (!classSplitCnt[bel])
						candidates.push_back(bel);
					++classSplitCnt[bel];
					classSplitElems[bel].push_back(x);
				}
			}
			for (int id : candidates) {
				if (classSplitCnt[id] != classSize[id]) {
					int jd = Split(id, classSplitElems[id]);
					for (int j = 0; j < m; ++j) {
						int Xid = jd;
						if (!inStack[id][j])
							Xid = classSize[id] < classSize[jd] ? id : jd;
						splitterStack[stackTop++] = {Xid, j};
						inStack[Xid][j] = 1u;
					}
				}
				classSplitCnt[id] = 0;
				classSplitElems[id].clear();
			}
		}
		inStack.clear();
		elemPosition.clear();
		classSplitElems.clear();
		classSplitCnt.clear();
		int classCnt = 0;
		std::vector<int> ret(n, -1);
		for (int i = 0; i < n; ++i) {
			if (ret[i] != -1)
				continue;
			int bel = elemBelong[i];
			for (int k = 0; k < classSize[bel]; ++k)
				ret[classElems[classBegin[bel] + k]] = classCnt;
			++classCnt;
		}
		return ret;
	}
public:
	std::vector<int> equivalenceClasses() const {
		if (!isComplete)
			RE("[Error] DFA::equivalenceClasses : Incomplete automaton.");
//		return equivalenceClassesTableFilling();
		return equivalenceClassesHopcroft();
	}
	DeterministicFiniteAutomaton eliminateUnreachableStates(bool clearName = false) const {
		if (!isComplete)
			RE("[Error] DFA::eliminateUnreachableStates : Incomplete automaton.");
		if (startStateIndex == -1)
			RE("[Error] DFA::eliminateUnreachableStates : Start state unset.");
		int m = sig.size();
		std::vector<unsigned char> reachable(n, 0u);
		std::vector<int> que(n);
		int head = 0, tail = 1;
		que[0] = startStateIndex;
		reachable[startStateIndex] = 1u;
		while (head != tail) {
			int u = que[head++];
			for (int j = 0; j < m; ++j) {
				int v = transitions[u][j];
				if (!reachable[v]) {
					reachable[v] = 1u;
					que[tail++] = v;
				}
			}
		}
		std::vector<int> map(n);
		map[0] = 0;
		for (int i = 1; i < n; ++i)
			map[i] = map[i - 1] + (int)reachable[i - 1];
		int newSize = map[n - 1] + (int)reachable[n - 1];
		DeterministicFiniteAutomaton ret(sig);
		ret.setSize(newSize);
		for (int u = 0; u < n; ++u) if (reachable[u]) {
			for (int j = 0; j < m; ++j) {
				int v = transitions[u][j];
				ret.transitions[map[u]][j] = map[v];
			}
		}
		ret.countEmptyTransitions = 0;
		ret.isComplete = true;
		for (int i = 0; i < n; ++i) if (reachable[i])
			ret.acceptStates[map[i]] = acceptStates[i];
		ret.startStateIndex = map[startStateIndex];
		if (!clearName) {
			for (int i = 0; i < n; ++i) if (reachable[i]) {
				if (!stateNames[i].empty())
					ret.stateNames[map[i]] = stateNames[i];
				else
					ret.stateNames[map[i]] = "q_" + std::to_string(i);
			}
		}
		return ret;
	}
	DeterministicFiniteAutomaton minimize(bool clearName = false) const {
		if (!isComplete)
			RE("[Error] DFA::minimize : Incomplete automaton.");
		if (startStateIndex == -1)
			RE("[Error] DFA::minimize : Start state unset.");
		int m = sig.size();
		DeterministicFiniteAutomaton A = eliminateUnreachableStates(clearName);
		int oldSize = A.size();
		std::vector<int> eqv = A.equivalenceClasses();
		int newSize = *std::max_element(eqv.begin(), eqv.end()) + 1;
		DeterministicFiniteAutomaton ret(sig);
		ret.setSize(newSize);
		std::vector<int> represent(newSize, -1);
		for (int i = 0; i < oldSize; ++i)
			if (represent[eqv[i]] == -1)
				represent[eqv[i]] = i;
		for (int i = 0; i < newSize; ++i) {
			int u = represent[i];
			for (int j = 0; j < m; ++j) {
				int v = A.transitions[u][j];
				ret.transitions[i][j] = eqv[v];
			}
		}
		ret.countEmptyTransitions = 0;
		ret.isComplete = true;
		for (int i = 0; i < newSize; ++i)
			ret.acceptStates[i] = A.acceptStates[represent[i]];
		ret.startStateIndex = eqv[A.startStateIndex];
		if (!clearName) {
			for (int i = 0; i < oldSize; ++i) {
				if (ret.stateNames[eqv[i]].empty())
					ret.stateNames[eqv[i]] = A.stateNames[i];
				else
					(ret.stateNames[eqv[i]] += ", ") += A.stateNames[i];
			}
		}
		return ret;
	}
	~DeterministicFiniteAutomaton() = default;
};
std::string DeterministicFiniteAutomaton::State::name() const {
	return this->DFA().getStateName(*this);
}
DeterministicFiniteAutomaton::State DeterministicFiniteAutomaton::State::transition(int symbol) const {
	return this->DFA().transition(*this, symbol);
}
DeterministicFiniteAutomaton::State DeterministicFiniteAutomaton::State::transition(const std::vector<int> &symbols) const {
	return this->DFA().transition(*this, symbols);
}
DeterministicFiniteAutomaton::State DeterministicFiniteAutomaton::State::transition(const std::string &symbString) const {
	return this->DFA().transition(*this, symbString);
}
bool DeterministicFiniteAutomaton::State::accept() const {
	return this->DFA().isAcceptState(*this);
}

#undef RE
#undef WARN

#endif
