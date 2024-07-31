

#include <map>

#include <cassert>
#include <iostream>

#define assert(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: (" #cond "), function " << __FUNCTION__ << ", file " << __FILE__ << ", line " << __LINE__ << ".\n"; \
            __debugbreak(); \
        } \
    } while (false)



#include <iostream>

class Key {
public:
    explicit Key(int value) : value(value) {}

    // Operator < pentru Key
    bool operator<(const Key& other) const {
        return value < other.value;
    }

private:
    int value;
};

class Value {
public:
    explicit Value(int value) : value(value) {}

    // Operator == pentru Value
    bool operator==(const Value& other) const {
        return value == other.value;
    }

private:
    int value;
};



template<typename K, typename V>
class interval_map {
	friend void IntervalMapTest();
	V m_valBegin;
	std::map<K, V> m_map;
public:
	// constructor associates whole range of K with val
	interval_map(V const& val)
		: m_valBegin(val)
	{}

    // look-up of the value associated with key
    V const& operator[](K const& key) const {
        auto it = m_map.upper_bound(key);
        if (it == m_map.begin()) {
            return m_valBegin;
        }
        else {
            return (--it)->second;
        }
    }

    void assign(K const& keyBegin, K const& keyEnd, V const& val) {
        if (!(keyBegin < keyEnd)) {
            return;
        }

        auto not_equal = [](const K& a, const K& b) -> bool {
            return a < b || b < a;
            };


        // Find iterators for keyBegin and keyEnd
        auto itLow = m_map.lower_bound(keyBegin);
        auto itHigh = m_map.lower_bound(keyEnd);

        // If itLow is not at the beginning, check if we need to update the value at keyBegin
        if (itLow != m_map.begin()) {
            auto prev = std::prev(itLow);
            if (prev->second == val) {
                // If the previous value is the same as the new value, no need to change itLow
                itLow = prev;
            }
        }

        // Remove intervals that intersect with the interval [keyBegin, keyEnd)
        auto it = itLow;
        while (it != itHigh) {
            auto next = std::next(it);
            m_map.erase(it);
            it = next;
            itLow = it;
        }

        // Add the value val at the start of the interval
        if (itLow == m_map.end()) {
            //m_map[keyBegin] = val;
            m_map.emplace(keyBegin, val);
        }
        else if (not_equal(itLow->first, keyBegin)) {
       // else if (itLow->first != keyBegin) {
            //m_map[keyBegin] = val;
            m_map.emplace(keyBegin, val);
        }

        // Add the default value at the end of the interval
        if (itHigh == m_map.end()) {
            //m_map[keyEnd] = m_valBegin;
            m_map.emplace(keyEnd, m_valBegin);
        }
        else {
            // Ensure the interval [keyBegin, keyEnd) has the correct value at keyEnd
            auto prevHigh = std::prev(std::prev(itHigh));
            if (prevHigh != m_map.end()) {
                //if (keyEnd != itHigh->first) {
                if (not_equal(keyEnd, itHigh->first)) {
                   // m_map[keyEnd] = prevHigh->second;
                    m_map.emplace(keyEnd, prevHigh->second);
                }
            }
            else {
                //m_map[keyEnd] = m_valBegin;
                m_map.emplace(keyEnd, m_valBegin);
            }
        }
    }




};


void testAssignBasic() {
    interval_map<int, char> im('A');
    im.assign(1, 3, 'B'); // Interval [1, 3) -> 'B'
    im.assign(5, 7, 'C'); // Interval [5, 7) -> 'C'

    assert(im[0] == 'A'); // înainte de 1
    assert(im[1] == 'B'); // în intervalul [1, 3)
    assert(im[2] == 'B'); // în intervalul [1, 3)
    assert(im[3] == 'A'); // după 3
    assert(im[4] == 'A'); // înainte de 5
    assert(im[5] == 'C'); // în intervalul [5, 7)
    assert(im[6] == 'C'); // în intervalul [5, 7)
    assert(im[7] == 'A'); // după 7


    {
        interval_map<int, char> im('A');
        // Setări de intervale
        im.assign(1, 10, 'E'); // Interval [1, 10) -> 'E'
        im.assign(7, 8, 'X');  // Interval [7, 8) -> 'X'

        // Verificări după setări
        assert(im[0] == 'A'); // înainte de 1
        assert(im[1] == 'E'); // în intervalul [1, 10)
        assert(im[2] == 'E'); // în intervalul [1, 10)
        assert(im[3] == 'E'); // în intervalul [1, 10)
        assert(im[4] == 'E'); // în intervalul [1, 10)
        assert(im[5] == 'E'); // în intervalul [1, 10)
        assert(im[6] == 'E'); // în intervalul [1, 10)
        assert(im[7] == 'X'); // în intervalul [7, 8)
        assert(im[8] == 'E'); // în intervalul [8, 10)
        assert(im[9] == 'E'); // în intervalul [8, 10)
        assert(im[10] == 'A'); // după 10
        assert(im[11] == 'A'); // după 10
        assert(im[100] == 'A'); // după 10
    }

}


void testAssignOverlapScenarios() {
    // Test 1: Suprapunere parțială și completă
    {
        interval_map<int, char> im('A');
        im.assign(1, 5, 'B'); // Interval [1, 5) -> 'B'
        im.assign(3, 7, 'C'); // Interval [3, 7) -> 'C'

        assert(im[0] == 'A'); // înainte de 1
        assert(im[1] == 'B'); // în intervalul [1, 5)
        assert(im[2] == 'B'); // în intervalul [1, 5)
        assert(im[3] == 'C'); // în intervalul [3, 7)
        assert(im[4] == 'C'); // în intervalul [3, 7)
        assert(im[5] == 'C'); // în intervalul [3, 7)
        assert(im[6] == 'A'); // după 7
        assert(im[7] == 'A'); // după 7
        assert(im[8] == 'A'); // după 7
    }

    // Test 2: Suprapunere completă și extindere
    {
        interval_map<int, char> im('A');
        im.assign(2, 6, 'B'); // Interval [2, 6) -> 'B'
        im.assign(1, 7, 'C'); // Interval [1, 7) -> 'C'

        assert(im[0] == 'A'); // înainte de 1
        assert(im[1] == 'C'); // în intervalul [1, 7)
        assert(im[2] == 'C'); // în intervalul [1, 7)
        assert(im[3] == 'C'); // în intervalul [1, 7)
        assert(im[4] == 'C'); // în intervalul [1, 7)
        assert(im[5] == 'C'); // în intervalul [1, 7)
        assert(im[6] == 'C'); // în intervalul [1, 7)
        assert(im[7] == 'A'); // după 7
    }

    // Test 3: Suprapunere parțială pe margini
    {
        interval_map<int, char> im('A');
        im.assign(2, 6, 'B'); // Interval [2, 6) -> 'B'
        im.assign(5, 8, 'C'); // Interval [5, 8) -> 'C'

        assert(im[0] == 'A'); // înainte de 2
        assert(im[1] == 'A'); // înainte de 2
        assert(im[2] == 'B'); // în intervalul [2, 5)
        assert(im[3] == 'B'); // în intervalul [2, 5)
        assert(im[4] == 'B'); // în intervalul [2, 5)
        assert(im[5] == 'C'); // în intervalul [5, 8)
        assert(im[6] == 'C'); // în intervalul [5, 8)
        assert(im[7] == 'C'); // în intervalul [5, 8)
        assert(im[8] == 'A'); // după 8
    }

    // Test 4: Interval în interiorul altui interval
    {
        interval_map<int, char> im('A');
        im.assign(1, 10, 'B'); // Interval [1, 10) -> 'B'
        im.assign(3, 7, 'C'); // Interval [3, 7) -> 'C'

        assert(im[0] == 'A'); // înainte de 1
        assert(im[1] == 'B'); // în intervalul [1, 10)
        assert(im[2] == 'B'); // în intervalul [1, 10)
        assert(im[3] == 'C'); // în intervalul [3, 7)
        assert(im[4] == 'C'); // în intervalul [3, 7)
        assert(im[5] == 'C'); // în intervalul [3, 7)
        assert(im[6] == 'C'); // în intervalul [3, 7)
        assert(im[7] == 'B'); // în intervalul [7, 10)
        assert(im[8] == 'B'); // în intervalul [7, 10)
        assert(im[9] == 'B'); // în intervalul [7, 10)
        assert(im[10] == 'A'); // după 10
    }

    // Test 5: Intervale multiple succesive
    {
        interval_map<int, char> im('A');
        im.assign(1, 3, 'B'); // Interval [1, 3) -> 'B'
        im.assign(4, 6, 'C'); // Interval [4, 6) -> 'C'
        im.assign(7, 9, 'D'); // Interval [7, 9) -> 'D'

        assert(im[0] == 'A'); // înainte de 1
        assert(im[1] == 'B'); // în intervalul [1, 3)
        assert(im[2] == 'B'); // în intervalul [1, 3)
        assert(im[3] == 'A'); // după 3
        assert(im[4] == 'C'); // în intervalul [4, 6)
        assert(im[5] == 'C'); // în intervalul [4, 6)
        assert(im[6] == 'A'); // după 6
        assert(im[7] == 'D'); // în intervalul [7, 9)
        assert(im[8] == 'D'); // în intervalul [7, 9)
        assert(im[9] == 'A'); // după 9
    }
}

void testAssignBoundaryChange() {
    interval_map<int, char> im('A');
    im.assign(2, 5, 'B'); // Interval [2, 5) -> 'B'
    im.assign(5, 7, 'C'); // Interval [5, 7) -> 'C'

    assert(im[0] == 'A'); // înainte de 2
    assert(im[1] == 'A'); // înainte de 2
    assert(im[2] == 'B'); // în intervalul [2, 5)
    assert(im[3] == 'B'); // în intervalul [2, 5)
    assert(im[4] == 'B'); // în intervalul [2, 5)
    assert(im[5] == 'C'); // în intervalul [5, 7)
    assert(im[6] == 'C'); // în intervalul [5, 7)
    assert(im[7] == 'A'); // după 7
}


void testAssignEmptyInterval() {
    interval_map<int, char> im('A');
    im.assign(3, 3, 'B'); // Interval [3, 3) -> nu face nimic

    assert(im[0] == 'A'); // înainte de 3
    assert(im[1] == 'A'); // înainte de 3
    assert(im[2] == 'A'); // înainte de 3
    assert(im[3] == 'A'); // intervalul gol
}



void testAssignExtendedToInfinity() {
    interval_map<int, char> im('A');
    im.assign(1, 5, 'B'); // Interval [1, 5) -> 'B'
    im.assign(5, std::numeric_limits<int>::max(), 'C'); // Interval [5, ∞) -> 'C'

    assert(im[0] == 'A'); // înainte de 1
    assert(im[1] == 'B'); // în intervalul [1, 5)
    assert(im[2] == 'B'); // în intervalul [1, 5)
    assert(im[3] == 'B'); // în intervalul [1, 5)
    assert(im[4] == 'B'); // în intervalul [1, 5)
    assert(im[5] == 'C'); // în intervalul [5, ∞)
    assert(im[6] == 'C'); // în intervalul [5, ∞)
    assert(im[1000] == 'C'); // în intervalul [5, ∞)
    // Poți adăuga teste pentru intervale mari pentru a verifica performanța
}

void testMultipleAssigns() {
    interval_map<int, char> im('A');

    // Test pentru intervale separate
    im.assign(1, 3, 'B'); // Interval [1, 3) -> 'B'
    im.assign(4, 6, 'C'); // Interval [4, 6) -> 'C'
    im.assign(8, 9, 'D'); // Interval [8, 9) -> 'D'

    // Verificarea valorilor inițiale
    assert(im[0] == 'A'); // înainte de 1
    assert(im[1] == 'B'); // în intervalul [1, 3)
    assert(im[2] == 'B'); // în intervalul [1, 3)
    assert(im[3] == 'A'); // după 3
    assert(im[4] == 'C'); // în intervalul [4, 6)
    assert(im[5] == 'C'); // în intervalul [4, 6)
    assert(im[6] == 'A'); // după 6
    assert(im[7] == 'A'); // între 6 și 8
    assert(im[8] == 'D'); // în intervalul [8, 9)
    assert(im[9] == 'A'); // după 9
    assert(im[10] == 'A'); // după 9

    // Adăugarea și suprapunerea intervalelor suplimentare
    im.assign(0, 2, 'D'); // Interval [0, 2) -> 'D'
    im.assign(1, 10, 'E'); // Interval [1, 10) -> 'E'
    im.assign(7, 8, 'F'); // Interval [7, 8) -> 'F'

    // Verificări după modificări
    assert(im[0] == 'D'); // Interval [0, 2) -> 'D'
    assert(im[1] == 'E'); // Interval [1, 10) -> 'E'
    assert(im[2] == 'E'); // Interval [1, 10) -> 'E'
    assert(im[3] == 'E'); // Interval [1, 10) -> 'E'
    assert(im[4] == 'E'); // Interval [1, 10) -> 'E'
    assert(im[5] == 'E'); // Interval [1, 10) -> 'E'
    assert(im[6] == 'E'); // Interval [1, 10) -> 'E'
    assert(im[7] == 'F'); // Interval [7, 8) -> 'F'
    assert(im[8] == 'E'); // Interval [8, 10) -> 'E'
    assert(im[9] == 'E'); // Interval [8, 10) -> 'E
    assert(im[10] == 'A'); // Interval [8, 10) -> 'E''
}



auto not_equal = [](const Key& a, const Key& b) -> bool {
    return a < b || b < a;
    };

int testkeyvaluse() {
    // Crearea unor instanțe de Key și Value
    Key k1(1), k2(2), k3(1);
    Value v1(10), v2(20), v3(10);

    // Testarea funcției not_equal
    std::cout << "Testing Key not_equal:" << std::endl;
    std::cout << "k1 and k2 are not equal: " << (not_equal(k1, k2) ? "true" : "false") << std::endl;
    std::cout << "k1 and k3 are equal: " << (not_equal(k1, k3) ? "true" : "false") << std::endl;

    // Testarea operatorului == pentru Value
    std::cout << "Testing Value equality:" << std::endl;
    std::cout << "v1 and v2 are not equal: " << (v1 == v2 ? "false" : "true") << std::endl;
    std::cout << "v1 and v3 are equal: " << (v1 == v3 ? "true" : "false") << std::endl;

    return 0;
}

void testAssignWithKeyValue() {
    interval_map<Key, Value> im(Value(0));  // Inițializarea cu valoarea implicită 0

    Key k1(1), k2(3), k3(5), k4(7);
    Value v1(10), v2(20), v3(30);

    im.assign(k1, k2, v1); // Interval [1, 3) -> 10
    im.assign(k3, k4, v2); // Interval [5, 7) -> 20

    // Testează valorile așteptate
    assert(im[Key(0)] == Value(0)); // înainte de 1, valoarea implicită
    assert(im[k1] == v1); // în intervalul [1, 3)
    assert(im[Key(2)] == v1); // în intervalul [1, 3)
    assert(im[k2] == Value(0)); // după 3, valoarea implicită
    assert(im[Key(4)] == Value(0)); // înainte de 5, valoarea implicită
    assert(im[k3] == v2); // în intervalul [5, 7)
    assert(im[Key(6)] == v2); // în intervalul [5, 7)
    assert(im[Key(8)] == Value(0)); // după 7, valoarea implicită

    // Testează suprapunerea intervalelor
    im.assign(k1, k4, v3); // Interval [1, 7) -> 30

    assert(im[k1] == v3); // în intervalul [1, 7)
    assert(im[Key(2)] == v3); // în intervalul [1, 7)
    assert(im[k3] == v3); // în intervalul [1, 7)
    assert(im[Key(5)] == v3); // în intervalul [1, 7)
    assert(im[Key(8)] == Value(0)); // după 7, valoarea implicită
}





int main()
{
	std::cout << "Hello World!\n";
    testAssignWithKeyValue();

    testkeyvaluse();

    testAssignBasic();
    testAssignBoundaryChange();
    testAssignEmptyInterval();
    testAssignExtendedToInfinity();
    testMultipleAssigns();

    std::cout << "End Hello World!\n";
}
