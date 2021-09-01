#include "DFA.h"

const Alphabet alphabet_01(2, [](int x) { return x + '0'; }, [](int x) { return x - '0'; });
const Alphabet alphabet_number(10, [](int x) { return x + '0'; }, [](int x) { return x - '0'; });
const Alphabet alphabet_abc(3, [](int x) { return x + 'a'; }, [](int x) { return x - 'a'; });
const Alphabet alphabet_latin(26, [](int x) { return x + 'a'; }, [](int x) { return x - 'a'; });

// a DFA minimize example: DFAs accept base 10 numbers which remainder is 0 modulo the input value
int main() {
	using DFA = DeterministicFiniteAutomaton;
	int modular;
	scanf("%d", &modular);
	DFA A(alphabet_number);
	A.setSize(modular);
	for (int i = 0; i < modular; ++i)
		for (int j = 0; j < 10; ++j)
			A.setTransition(A[i], '0' + j, A[(i * 10 + j) % modular]);
	A.setStartState(A[0]);
	A.flipStateAcceptance(A[0]);
	printf("Is A complete : %s.\n", A.complete() ? "Yes" : "No");
	for (int i = 1; i <= 33 * modular; ++i) {
		std::string stri = std::to_string(i);
		DFA::State st = A.start().transition(stri);
		if ((i % modular == 0) ^ st.accept())
			printf("%9d : %s : %s\n", i, st.name().c_str(), st.accept() ? "accept" : "reject");
	}
	DFA B = A.minimize();
	printf("Is B complete : %s.\n", B.complete() ? "Yes" : "No");
	printf("B.size() = %d\n", B.size());
	for (int i = 1; i <= 33 * modular; ++i) {
		std::string stri = std::to_string(i);
		DFA::State st = B.start().transition(stri);
		if ((i % modular == 0) ^ st.accept())
			printf("%9d : %s : %s\n", i, st.name().c_str(), st.accept() ? "accept" : "reject");
	}
	for (int i = 0; i < B.size(); ++i)
		printf("B[%d].name() = %s\n", i, B[i].name().c_str());
	return 0;
}
