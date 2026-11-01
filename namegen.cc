/**
 *
 * @file A fantasy name generator library.
 * @version 1.0.1
 * @license Public Domain
 * @author German M. Bravo (Kronuz)
 *
 */

#include "namegen.h"

#include <algorithm>  // for reverse, min, max
#include <chrono>     // for rng seed
#include <cstdint>    // for uint32_t, uint64_t
#include <cstring>    // for memcpy
#include <cwchar>     // for size_t, mbsrtowcs, wcsrtombs
#include <cwctype>    // for towupper
#include <functional> // for hash
#include <iterator>   // for iterator_traits
#include <memory>     // for make_unique
#include <random>     // for mt19937
#include <stdexcept>  // for invalid_argument
#include <thread>     // for this_thread::get_id
#include <typeinfo>   // for typeid


using namespace NameGen;


// One engine per thread: concurrent toString() calls never share one. The thread's
// id joins the clock in the seed so threads that start together still differ.
std::mt19937& NameGen::engine()
{
	thread_local std::mt19937 rng(static_cast<std::mt19937::result_type>(
		static_cast<unsigned long long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) ^
		std::hash<std::thread::id>()(std::this_thread::get_id())));
	return rng;
}



// https://isocpp.org/wiki/faq/ctors#static-init-order
// Avoid the "static initialization order fiasco"
const std::unordered_map<std::string, const std::vector<std::string>>& Generator::SymbolMap()
{
	static auto* const symbols = new std::unordered_map<std::string, const std::vector<std::string>>({
		{
			"s", {
				"ach", "ack", "ad", "age", "ald", "ale", "an", "ang", "ar", "ard",
				"as", "ash", "at", "ath", "augh", "aw", "ban", "bel", "bur", "cer",
				"cha", "che", "dan", "dar", "del", "den", "dra", "dyn", "ech", "eld",
				"elm", "em", "en", "end", "eng", "enth", "er", "ess", "est", "et",
				"gar", "gha", "hat", "hin", "hon", "ia", "ight", "ild", "im", "ina",
				"ine", "ing", "ir", "is", "iss", "it", "kal", "kel", "kim", "kin",
				"ler", "lor", "lye", "mor", "mos", "nal", "ny", "nys", "old", "om",
				"on", "or", "orm", "os", "ough", "per", "pol", "qua", "que", "rad",
				"rak", "ran", "ray", "ril", "ris", "rod", "roth", "ryn", "sam",
				"say", "ser", "shy", "skel", "sul", "tai", "tan", "tas", "ther",
				"tia", "tin", "ton", "tor", "tur", "um", "und", "unt", "urn", "usk",
				"ust", "ver", "ves", "vor", "war", "wor", "yer"
			}
		},
		{
			"S", {
				"ba", "bai", "bau", "be", "bi", "bia", "bio", "bla", "blo", "blu", "bo", "bra",
				"brau", "bri", "bria", "brie", "bro", "bru", "bue", "ca",
				"ce", "cha", "che", "chi", "cho", "chu", "ci", "cia", "cie", "cio", "cla",
				"cle", "cli", "clo", "co", "cra", "cre", "cri", "cu", "cua", "da", "dai",
				"daia", "de", "dei", "di", "dia", "die", "dio",
				"do", "doi", "doia", "dou", "dra", "dre", "dri", "dria", "dro", "du", "dua",
				"dui", "fa", "fe", "fi", "fia", "fie", "fio", "fla", "flo", "fo", "fra",
				"fre", "fri", "fu", "ga", "gau", "ge", "gei", "gi", "gia", "gie", "gio",
				"giu", "gla", "glai", "gle", "glei", "glo", "go", "gra", "gre", "gri", "gu",
				"gua", "gue", "ja", "jai", "je",
				"ji", "jo", "ju", "jua", "la", "lai", "le", "lei",
				"leu", "li", "lia", "lie", "lio", "liu",
				"lo", "loi", "lu", "lua", "lui", "ma", "mai",
				"maia", "mau", "me", "mei", "mi", "mia", "mla", "mo", "moi",
				"mu", "na", "nai", "naia", "ne", "nei", "neu", "ni", "nia", "nie",
				"nio", "no", "nu", "nue", "pa", "pau", "pe", "peyo",
				"pi", "pia", "pie", "po", "pra", "pri", "pria", "pu", "que",
				"qui", "ra", "rai", "raya", "re", "rei", "rey", "ri", "ria",
				"rie", "rio", "rla", "rle", "rli", "rlo", "ro",
				"ru", "sa", "sai", "sau", "sca", "se", "si", "sia", "sio", "so",
				"ste", "su", "sue", "ta",
				"tai", "taua", "te", "ti", "tia", "tla", "to",
				"tra", "tre", "tri", "trio", "tro", "troi", "tru", "tu",
				"va", "vai", "ve", "vei", "vi", "via", "vie", "vio", "vo",
				"za", "zai", "ze", "zi", "zia", "zie", "zio", "zo",
			}
		},
		{
			"b", {
				"bal", "ban", "bar", "bas", "bed", "bel", "ben", "ber", "bet", "bian", "bil",
				"bin", "blan", "blas", "bles", "bob", "bon", "bran", "bras", "brec", "bren", "bril",
				"bur", "can", "car", "cas", "cat", "cay", "ced", "cel", "cen", "cep", "ces", "chan",
				"ches", "chor", "cial", "cin", "cion", "cir", "cis", "claud", "cles", "clif", "cob",
				"com", "con", "cons", "cor", "cos", "cosm", "cost", "cris", "cual", "cun", "cus", "dad",
				"daf", "dal", "dam", "dan", "dar", "das", "del", "den", "der", "des", "det",
				"dieg", "dic", "diel", "dier", "dil", "din", "dios", "dir", "dis", "dit", "dol",
				"don", "dor", "dox", "dras", "dred", "dres", "duar", "dul", "dun",
				"fal", "fan", "faus", "fer", "fet", "fin", "flor", "fon", "fran", "fred",
				"fren", "fris", "ful", "gail", "gan", "gar", "gas", "gel", "gels", "gem", "gen",
				"ger", "ges", "gian", "gib", "gil", "gin", "gior", "giot", "gis", "git", "glas",
				"glen", "gon", "gor", "gos", "got", "gret", "grid", "gros", "gual", "guel",
				"guer", "gun", "gus", "guz", "jac",
				"jan", "jas", "jauc", "jaz", "jef", "jen", "jes", "jis", "jor", "jos", "juan",
				"jur", "jus", "lain", "lais", "lam", "lan", "lar", "las", "laur", "leb", "led",
				"lem", "len", "ler", "les", "let", "liam", "lian", "liel", "liet", "lil",
				"lim", "lin", "lins", "lip", "lir", "lis", "lius", "liz", "lon", "lor", "lot",
				"luc", "lud", "luis", "lum", "lus", "lux", "luz", "mad", "mag", "mal", "man",
				"mar", "mas", "mat", "max", "med", "mel", "men", "mer", "mes",
				"mian", "mic", "mig", "mil", "mim", "min", "mir", "mis", "mit", "mon", "mor",
				"mos", "muel", "mun", "mus", "nac", "nal", "nan", "nar",
				"nas", "nat", "nef", "nel", "nep", "ner", "nes", "net", "neus", "nic", "nid",
				"niel", "nif", "nil", "nin", "nis", "noc", "nol", "non", "nor", "nos",
				"nuar", "nuel", "nuem", "nul", "nun", "nur", "nus", "pan", "par", "pas",
				"paz", "peb", "peg", "per", "pol", "pom", "pris", "prit", "quar", "quel",
				"ques", "quet", "quin", "ral", "ram", "ran", "rap", "rar", "ras", "raz",
				"rel", "rem", "ren", "res", "ret", "ric", "rid", "riel",
				"riet", "ril", "rim", "rin", "riol", "ris", "rit", "sac",
				"sal", "sam", "san", "sar", "sas", "sef", "sel", "sen", "sep", "ser", "ses",
				"set", "siel", "sier", "sig", "sil", "sin", "sion", "sis", "six",
				"sol", "son", "sop", "sual", "sun", "sus", "tac", "tad", "tal",
				"tan", "tap", "tas", "tel", "ten", "ter", "tes", "tian", "tif",
				"til", "tin", "tir", "tis", "ton", "top", "tor", "tos", "tr", "tran", "trid",
				"tris", "triz", "val", "van", "var", "vas", "vec", "ven", "ver", "ves",
				"vian", "vic", "vid", "vier", "vil", "vin", "vir", "vis", "von", "vor", "vril",
				"z", "zac", "zaid", "zan", "zar", "zel", "zen", "zer", "zid", "ziel",
				"zul",
			}
		},
		{
			"v", {
				"a", "e", "i", "o", "u", "y"
			}
		},
		{
			"V", {
				"a", "e", "i", "o", "u", "y", "ae", "ai", "au", "ay", "ea", "ee",
				"ei", "eu", "ey", "ia", "ie", "oe", "oi", "oo", "ou", "ui"
			}
		},
		{
			"c", {
				"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r",
				"s", "t", "v", "w", "x", "y", "z"
			}
		},
		{
			"B", {
				"b", "bl", "br", "c", "ch", "chr", "cl", "cr", "d", "dr", "f", "g",
				"h", "j", "k", "l", "ll", "m", "n", "p", "ph", "qu", "r", "rh", "s",
				"sch", "sh", "sl", "sm", "sn", "st", "str", "sw", "t", "th", "thr",
				"tr", "v", "w", "wh", "y", "z", "zh"
			}
		},
		{
			"C", {
				"b", "c", "ch", "ck", "d", "f", "g", "gh", "h", "k", "l", "ld", "ll",
				"lt", "m", "n", "nd", "nn", "nt", "p", "ph", "q", "r", "rd", "rr",
				"rt", "s", "sh", "ss", "st", "t", "th", "v", "w", "y", "z"
			}
		},
		{
			"i", {
				"air", "ankle", "ball", "beef", "bone", "bum", "bumble", "bump",
				"cheese", "clod", "clot", "clown", "corn", "dip", "dolt", "doof",
				"dork", "dumb", "face", "finger", "foot", "fumble", "goof",
				"grumble", "head", "knock", "knocker", "knuckle", "loaf", "lump",
				"lunk", "meat", "muck", "munch", "nit", "numb", "pin", "puff",
				"skull", "snark", "sneeze", "thimble", "twerp", "twit", "wad",
				"wimp", "wipe"
			}
		},
		{
			"m", {
				"baby", "booble", "bunker", "cuddle", "cuddly", "cutie", "doodle",
				"foofie", "gooble", "honey", "kissie", "lover", "lovey", "moofie",
				"mooglie", "moopie", "moopsie", "nookum", "poochie", "poof",
				"poofie", "pookie", "schmoopie", "schnoogle", "schnookie",
				"schnookum", "smooch", "smoochie", "smoosh", "snoogle", "snoogy",
				"snookie", "snookum", "snuggy", "sweetie", "woogle", "woogy",
				"wookie", "wookum", "wuddle", "wuddly", "wuggy", "wunny"
			}
		},
		{
			"M", {
				"boo", "bunch", "bunny", "cake", "cakes", "cute", "darling",
				"dumpling", "dumplings", "face", "foof", "goo", "head", "kin",
				"kins", "lips", "love", "mush", "pie", "poo", "pooh", "pook", "pums"
			}
		},
		{
			"D", {
				"b", "bl", "br", "cl", "d", "f", "fl", "fr", "g", "gh", "gl", "gr",
				"h", "j", "k", "kl", "m", "n", "p", "th", "w"
			}
		},
		{
			"d", {
				"elch", "idiot", "ob", "og", "ok", "olph", "olt", "omph", "ong",
				"onk", "oo", "oob", "oof", "oog", "ook", "ooz", "org", "ork", "orm",
				"oron", "ub", "uck", "ug", "ulf", "ult", "um", "umb", "ump", "umph",
				"un", "unb", "ung", "unk", "unph", "unt", "uzz"
			}
		}
	});

	return *symbols;
}


// Use std::make_unique when the compiler provides it (C++14 and later), detected
// via __cplusplus as well as the explicit HAVE_CXX14. Relying on HAVE_CXX14 alone
// is a footgun on a C++14+ build that does not define it: the unqualified
// make_unique calls below find std::make_unique by ADL on their std:: arguments,
// which then clashes with the C++11 fallback ("call to 'make_unique' is ambiguous").
#if defined(HAVE_CXX14) || __cplusplus >= 201402L
using std::make_unique;
#else
// make_unique is not available in c++11, so we use this template function
// to maintain full c++11 compatibility; std::make_unique is part of C++14.
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif


namespace {

// Counts that do not fit in a size_t saturate at its maximum rather than wrap.
size_t saturating_add(size_t a, size_t b)
{
	return a > static_cast<size_t>(-1) - b ? static_cast<size_t>(-1) : a + b;
}


size_t saturating_multiply(size_t a, size_t b)
{
	return a != 0 && b > static_cast<size_t>(-1) / a ? static_cast<size_t>(-1) : a * b;
}


// A choice among n >= 2 alternatives, each with probability 1/n, from 32-bit outputs
// of the engine: the high half of x * n (Lemire, "Fast Random Integer Generation in an
// Interval", 2019), drawing again when the low half falls among the 2^32 mod n values
// that would favor some alternatives. A choice among one alternative draws nothing.
template <typename Engine>
inline uint32_t pick(Engine& e, uint32_t n)
{
	uint64_t m = static_cast<uint64_t>(static_cast<uint32_t>(e())) * n;
	uint32_t low = static_cast<uint32_t>(m);
	if (low < n) {
		uint32_t threshold = static_cast<uint32_t>(0u - n) % n;
		while (low < threshold) {
			m = static_cast<uint64_t>(static_cast<uint32_t>(e())) * n;
			low = static_cast<uint32_t>(m);
		}
	}
	return static_cast<uint32_t>(m >> 32);
}


template <typename Engine>
inline size_t choose(Engine& e, size_t n)
{
	return n < 2 ? 0 : pick(e, static_cast<uint32_t>(n));
}


// Where a name is written: [0, pos) of p is the text so far and [pos, cap) room for
// more. It is either a std::string (the room is the string's size past pos, given
// back at finish()) or a caller's local buffer that moves to a std::string of its own
// when it fills. `ascii` says that every byte written is ASCII 1 to 127.
struct Out {
	char* p;
	size_t pos;
	size_t cap;
	std::string* back;  // the string p points into, nullptr for a local buffer
	std::string spill;  // the local buffer's text once it outgrows it
	bool ascii = false;

	explicit Out(std::string& s) : p(&s[0]), pos(s.size()), cap(s.size()), back(&s) {}
	Out(char* local, size_t n) : p(local), pos(0), cap(n), back(nullptr) {}
	Out(const Out&) = delete;
	Out& operator=(const Out&) = delete;

	void grow(size_t n)
	{
		size_t want = std::max(pos + n, 2 * cap);
		if (!back) {
			spill.assign(p, pos);
			back = &spill;
		}
		back->resize(want);
		p = &(*back)[0];
		cap = want;
	}

	void reserve(size_t n)
	{
		if (pos + n > cap) {
			grow(n);
		}
	}

	void put(const char* src, size_t n)
	{
		reserve(n);
		std::memcpy(p + pos, src, n);
		pos += n;
	}

	void put(char c)
	{
		reserve(1);
		p[pos++] = c;
	}

	// A std::string Out gives its unused room back.
	void finish()
	{
		if (back && back != &spill) {
			back->resize(pos);
		}
	}
};


// Reverser, Capitalizer and Collapser work on characters: they decode the bytes with
// the C library's multibyte functions (in the current locale), transform, and encode
// back; bytes the locale does not decode give "", and a NUL ends the text. Bytes 1 to
// 127 are ASCII, which decodes to itself in every locale the C libraries ship (each
// is a superset of ASCII), so a text of only those bytes is transformed in place, with
// no conversion and no allocation, and gives what the conversion gives.
bool plain(const char* p, size_t n)
{
	// Branch-free so that it vectorizes: c - 1 wraps NUL to 255.
	unsigned char bad = 0;
	for (size_t i = 0; i < n; ++i) {
		bad |= static_cast<unsigned char>(static_cast<unsigned char>(p[i]) - 1) >= 0x7f;
	}
	return !bad;
}


bool plain(const Out& out, size_t from)
{
	return out.ascii || plain(out.p + from, out.pos - from);
}


// Replaces out[from, pos) by f(that text, decoded), encoded back.
template <typename F>
void transform_decoded(Out& out, size_t from, F f)
{
	std::wstring str = towstring(std::string(out.p + from, out.pos - from));
	f(str);
	std::string back = tostring(str);
	out.pos = from;
	out.put(back.data(), back.size());
	out.ascii = false;
}


// Keeps at most one of a run of a, h, i, j, q, u, v, w, x or y and at most two of a
// run of anything else, writing over [first, last); returns the new end.
// Bytes kept of a run: one of a, h, i, j, q, u, v, w, x or y, two of anything else.
struct Keep {
	unsigned char n[256];
	Keep()
	{
		for (int c = 0; c < 256; ++c) {
			n[c] = 2;
		}
		for (const char* c = "ahijquvwxy"; *c; ++c) {
			n[static_cast<unsigned char>(*c)] = 1;
		}
	}
};


char* collapse(char* first, char* last)
{
	static const Keep keep;
	char* out = first;
	int cnt = 0;
	char pch = 0;
	for (char* it = first; it != last; ++it) {
		char ch = *it;
		cnt = ch == pch ? cnt + 1 : 0;
		*out = ch;
		out += cnt < keep.n[static_cast<unsigned char>(ch)];
		pch = ch;
	}
	return out;
}


template <typename It>
It collapse(It first, It last)
{
	It out = first;
	int cnt = 0;
	typename std::iterator_traits<It>::value_type pch = 0;
	for (It it = first; it != last; ++it) {
		auto ch = *it;
		if (ch == pch) {
			cnt++;
		} else {
			cnt = 0;
		}
		int mch = 2;
		switch(ch) {
			case 'a':
			case 'h':
			case 'i':
			case 'j':
			case 'q':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
				mch = 1;
		}
		if (cnt < mch) {
			*out++ = ch;
		}
		pch = ch;
	}
	return out;
}


void reverse_text(Out& out, size_t from)
{
	if (out.pos == from) {
		return;
	}
	char* p = out.p;
	if (plain(out, from)) {
		std::reverse(p + from, p + out.pos);
		return;
	}
	transform_decoded(out, from, [](std::wstring& str) {
		std::reverse(str.begin(), str.end());
	});
}


void capitalize_text(Out& out, size_t from)
{
	if (out.pos == from) {
		return;
	}
	char* p = out.p;
	if (plain(out, from)) {
		// towupper() can leave ASCII (a Turkish locale maps i to U+0130): then the
		// encoding takes over.
		std::wint_t upper = std::towupper(static_cast<unsigned char>(p[from]));
		if (upper > 0 && upper <= 0x7f) {
			p[from] = static_cast<char>(upper);
			return;
		}
	}
	transform_decoded(out, from, [](std::wstring& str) {
		if (!str.empty()) {
			str[0] = std::towupper(str[0]);
		}
	});
}


void collapse_text(Out& out, size_t from)
{
	if (out.pos == from) {
		return;
	}
	char* p = out.p;
	if (plain(out, from)) {
		// Only a byte equal to the one before it can go; collapse from the first such pair
		// (the byte before the pair differs from it, so the run starts there).
		char* end = p + out.pos;
		for (char* it = p + from + 1; it < end; ++it) {
			if (*it == it[-1]) {
				out.pos = static_cast<size_t>(collapse(it - 1, end) - p);
				return;
			}
		}
		return;
	}
	transform_decoded(out, from, [](std::wstring& str) {
		str.erase(collapse(str.begin(), str.end()), str.end());
	});
}


// Writes copy whole 16-byte blocks: a copy of n bytes reads and writes up to 15 past
// them, so its source has that much readable past its end and the output that much room.
const size_t kSlack = 16;

inline void copy_blocks(char* to, const char* from, size_t n)
{
	for (size_t k = 0; k < n; k += 16) {
		std::memcpy(to + k, from + k, 16);
	}
}


// The symbol tables, built once from SymbolMap(): each table's words back to back
// (followed by kSlack NULs), with the offset where each word begins and, at n, the
// end of the last.
struct Table {
	const char* text = nullptr;
	const uint32_t* begins = nullptr;
	uint32_t n = 0;
	size_t min = 0;
	size_t max = 0;
};


const Table* symbol_tables()
{
	// Built once and never destroyed (names can be drawn during static destruction);
	// `words` and `begins` hold what the tables point into.
	struct Tables {
		Table table[256];
		std::string words[256];
		std::vector<uint32_t> begins[256];
	};
	static const Table* const tables = [] {
		Tables* all = new Tables;
		for (const auto& entry : Generator::SymbolMap()) {
			if (entry.first.size() != 1 || entry.second.empty()) {
				continue;
			}
			unsigned char c = static_cast<unsigned char>(entry.first[0]);
			Table& table = all->table[c];
			std::string& words = all->words[c];
			std::vector<uint32_t>& begins = all->begins[c];
			table.min = static_cast<size_t>(-1);
			for (const auto& w : entry.second) {
				begins.push_back(static_cast<uint32_t>(words.size()));
				words += w;
				table.min = std::min(table.min, w.size());
				table.max = std::max(table.max, w.size());
			}
			begins.push_back(static_cast<uint32_t>(words.size()));
			words.append(kSlack, '\0');
			table.text = words.data();
			table.begins = begins.data();
			table.n = static_cast<uint32_t>(entry.second.size());
		}
		return all->table;
	}();
	return tables;
}


// The characters of a pattern, by what they do at the top level and inside <>, and
// inside (). Wrappers and symbols are characters inside ().
// The order matters: components (CHAR, SYMBOL) first, then wrappers, then what ends
// a run of them (BAR and the brackets).
enum : unsigned char { CHAR, SYMBOL, CAPITALIZE, REVERSE, BAR, OPEN_SYMBOLS, OPEN_LITERAL, CLOSE_SYMBOLS, CLOSE_LITERAL };

struct Classes {
	unsigned char symbols[256];
	unsigned char literal[256];

	Classes()
	{
		const Table* tables = symbol_tables();
		for (int c = 0; c < 256; ++c) {
			symbols[c] = tables[c].n ? SYMBOL : CHAR;
			literal[c] = CHAR;
		}
		const unsigned char both[][2] = {{'|', BAR}, {'<', OPEN_SYMBOLS}, {'(', OPEN_LITERAL},
		                                 {'>', CLOSE_SYMBOLS}, {')', CLOSE_LITERAL}};
		for (auto& b : both) {
			symbols[b[0]] = literal[b[0]] = b[1];
		}
		symbols[static_cast<unsigned char>('!')] = CAPITALIZE;
		symbols[static_cast<unsigned char>('~')] = REVERSE;
	}
};


const Classes& classes()
{
	static const Classes c;
	return c;
}


// A stack of trivially copyable values, in place up to N, on the heap past it.
template <typename T, size_t N>
class Stack {
	T local[N];
	std::unique_ptr<T[]> heap;
	T* d = local;
	size_t n = 0;
	size_t cap = N;

public:
	Stack() = default;
	Stack(const Stack&) = delete;
	Stack& operator=(const Stack&) = delete;

	void push(const T& v)
	{
		if (n == cap) {
			std::unique_ptr<T[]> bigger(new T[2 * cap]);
			std::copy(d, d + n, bigger.get());
			heap = std::move(bigger);
			d = heap.get();
			cap *= 2;
		}
		d[n++] = v;
	}

	void append(const T* first, const T* last)
	{
		for (; first != last; ++first) {
			push(*first);
		}
	}

	T& top() { return d[n - 1]; }
	T* data() { return d; }
	const T* data() const { return d; }
	T& operator[](size_t i) { return d[i]; }
	const T& operator[](size_t i) const { return d[i]; }
	void pop() { --n; }
	size_t size() const { return n; }
	void resize(size_t k) { n = k; }
};


const uint32_t NONE = 0xffffffffu;


// A group of a pattern: the top level, a <> or a (). Groups are numbered in the
// order their opening brackets appear, the top level 0. `end` is the position of
// the closing bracket (the pattern's length for the top level), `after` the number
// of the first group past this one's.
struct Group {
	uint32_t end;
	uint32_t after;
	uint32_t alts;       // alternatives, for a compiled pattern
	uint32_t first_alt;  // its first alternative in the alternatives array
};


// An alternative of a compiled group: where it starts, where the wrappers it carries
// in start (NONE when none; `!|s` capitalizes the s), and the number of the first
// group inside it.
struct Alt {
	uint32_t start;
	uint32_t carry;
	uint32_t first_group;
};


// The pattern scan shared by compiling and the one-shot outline: it checks the
// brackets as the original parser did (the same errors, in the same order) and hands
// what it meets to a recorder.
template <typename Recorder>
void scan(const char* p, uint32_t n, Recorder& r)
{
	const Classes& cls = classes();
	const Table* tables = symbol_tables();
	r.open(0, false, 0);
	bool literal = false;
	for (uint32_t i = 0; i < n; ++i) {
		const unsigned char* classes_here = literal ? cls.literal : cls.symbols;
		unsigned char k = classes_here[static_cast<unsigned char>(p[i])];
		switch (k) {
			case OPEN_SYMBOLS:
			case OPEN_LITERAL:
				literal = k == OPEN_LITERAL;
				r.open(i, literal, i + 1);
				break;
			case CLOSE_SYMBOLS:
			case CLOSE_LITERAL:
				if (r.depth() == 1) {
					throw std::invalid_argument("Unbalanced brackets");
				}
				if (k == CLOSE_SYMBOLS && literal) {
					throw std::invalid_argument("Unexpected '>' in pattern");
				}
				if (k == CLOSE_LITERAL && !literal) {
					throw std::invalid_argument("Unexpected ')' in pattern");
				}
				literal = r.close(i);
				break;
			case BAR:
				r.bar(i);
				break;
			case CAPITALIZE:
			case REVERSE:
				r.wrapper(i);
				break;
			case SYMBOL:
				if (Recorder::counts) {
					r.symbol(tables[static_cast<unsigned char>(p[i])]);
					break;
				}
				// Without counting, a symbol is a component like a character.
				// fall through
			default:
				r.character();
				if (!Recorder::counts) {
					// A run of characters and symbols is one component.
					while (i + 1 < n && classes_here[static_cast<unsigned char>(p[i + 1])] <= SYMBOL) {
						++i;
					}
				}
				break;
		}
	}
	if (r.depth() != 1) {
		throw std::invalid_argument("Missing closing bracket");
	}
	r.close(n);
}


// Records a pattern's groups with their alternatives (an alternative's records are
// contiguous, in the order of its group's), in place up to the sizes below and on the
// heap past them. With Count it also counts combinations(), min() and max() as the
// original's tree counted them (a choice sums, a sequence multiplies, saturating at
// SIZE_MAX); the one-shot path records without counting.
template <bool Count>
struct Recorder {
	static const bool counts = Count;

	struct Frame {
		uint32_t group;
		uint32_t content;   // where the group's first alternative starts
		uint32_t pending;   // the first wrapper waiting for a component, or NONE
		uint32_t count;     // alternatives so far
		uint32_t alts_base; // its alternatives in open_alts
		bool literal;
		size_t c, lo, hi;       // the alternative being read: count, shortest, longest
		size_t sum, min, max;   // the alternatives before it
	};

	Stack<Group, 64> groups;
	Stack<Alt, 256> alts;
	Stack<Frame, 32> frames;
	Stack<Alt, 256> open_alts;
	size_t c = 1, lo = 0, hi = 0;

	size_t depth() const { return frames.size(); }

	void begin_alt(Frame& f, uint32_t start, uint32_t carry)
	{
		open_alts.push(Alt{start, carry, static_cast<uint32_t>(groups.size())});
		++f.count;
		if (Count) {
			f.c = 1;
			f.lo = f.hi = 0;
		}
	}

	void component(Frame& f)
	{
		if (f.count == 0) {
			begin_alt(f, f.content, NONE);
		}
		f.pending = NONE;
	}

	void end_alt(Frame& f)
	{
		if (Count) {
			f.sum = saturating_add(f.sum, f.c);
			f.min = std::min(f.min, f.lo);
			f.max = std::max(f.max, f.hi);
		}
	}

	void open(uint32_t, bool literal, uint32_t content)
	{
		if (frames.size()) {
			component(frames.top());
		}
		uint32_t g = static_cast<uint32_t>(groups.size());
		groups.push(Group{0, 0, 0, 0});
		frames.push(Frame{g, content, NONE, 0, static_cast<uint32_t>(open_alts.size()), literal, 1, 0, 0, 0, static_cast<size_t>(-1), 0});
	}

	bool close(uint32_t i)
	{
		Frame f = frames.top();
		frames.pop();
		size_t gc = 1, glo = 0, ghi = 0;  // an empty group is Literal("")
		if (f.count) {
			end_alt(f);
			gc = f.sum ? f.sum : 1;
			glo = f.min;
			ghi = f.max;
		}
		Group& g = groups[f.group];
		g.end = i;
		g.after = static_cast<uint32_t>(groups.size());
		g.alts = f.count;
		g.first_alt = static_cast<uint32_t>(alts.size());
		alts.append(open_alts.data() + f.alts_base, open_alts.data() + open_alts.size());
		open_alts.resize(f.alts_base);
		if (!frames.size()) {
			c = gc;
			lo = glo;
			hi = ghi;
			return false;
		}
		Frame& parent = frames.top();
		if (Count) {
			parent.c = saturating_multiply(parent.c, gc);
			parent.lo += glo;
			parent.hi += ghi;
		}
		return parent.literal;
	}

	void bar(uint32_t i)
	{
		Frame& f = frames.top();
		if (f.count == 0) {
			begin_alt(f, f.content, NONE);
		}
		end_alt(f);
		begin_alt(f, i + 1, f.pending);  // the waiting wrappers carry over
	}

	void wrapper(uint32_t i)
	{
		Frame& f = frames.top();
		if (f.pending == NONE) {
			f.pending = i;
		}
	}

	void symbol(const Table& t)
	{
		Frame& f = frames.top();
		component(f);
		if (Count) {
			f.c = saturating_multiply(f.c, t.n);
			f.lo += t.min;
			f.hi += t.max;
		}
	}

	void character()
	{
		Frame& f = frames.top();
		component(f);
		if (Count) {
			f.lo += 1;
			f.hi += 1;
		}
	}
};


// Pushes the wrappers an alternative carries in: the ! and ~ between `carry` (the
// first of them) and the alternative's start, past the bars between them.
template <typename Pending>
void replay(const char* p, uint32_t carry, uint32_t start, Pending& pending)
{
	for (uint32_t q = carry; q < start; ++q) {
		if (p[q] == '!') {
			pending.push(CAPITALIZE);
		} else if (p[q] == '~') {
			pending.push(REVERSE);
		}
	}
}


// A recorded pattern, as the executor reads it: `readable` bytes of p can be read
// (kSlack more than the pattern when it was copied with padding).
struct View {
	const char* p;
	size_t readable;
	const Group* groups;
	const Alt* alts;
	const Classes& cls;
	const Table* tables;
};


void apply(unsigned char wrapper, Out& out, size_t from)
{
	if (wrapper == CAPITALIZE) {
		capitalize_text(out, from);
	} else {
		reverse_text(out, from);
	}
}


// Applies the wrappers waiting above `base` to out[from, pos), innermost (the last
// one written) first, and clears them.
template <typename Pending>
void wrap(Pending& pending, size_t base, Out& out, size_t from)
{
	for (size_t w = pending.size(); w-- > base; ) {
		apply(pending[w], out, from);
	}
	pending.resize(base);
}


struct Frame {
	uint32_t end;
	uint32_t after;
	uint32_t wrap_base;     // the wrappers applying to this group: pending[wrap_base, pending_base)
	uint32_t pending_base;  // this group's own waiting wrappers start here
	size_t out_start;
	const unsigned char* k; // its character classes
};


// Writes one name of a recorded pattern: walks the pattern from the chosen alternative
// of the top level, choosing an alternative at every group it enters and a word at
// every symbol, and skipping the rest. `choice` is the top level's alternative to take
// (NONE to draw it). The walk keeps the output position and the group it is in in
// locals and writes whole blocks (copy_blocks), keeping kSlack bytes of room.
template <typename Engine>
void execute(const View& v, Out& out, Engine& e, uint32_t choice)
{
	const Group* groups = v.groups;
	uint32_t top = groups[0].alts;
	if (top == 0) {
		return;
	}
	const char* p = v.p;
	const Alt* alts = v.alts;
	const Table* tables = v.tables;
	const unsigned char* const symbols = v.cls.symbols;
	const unsigned char* const literal = v.cls.literal;
	Stack<Frame, 32> frames;
	Stack<unsigned char, 32> pending;

	char* b = out.p;
	size_t o = out.pos;
	size_t lim = out.cap > kSlack ? out.cap - kSlack : 0;
	auto room = [&](size_t n) {
		if (o + n > lim) {
			out.pos = o;
			out.grow(n + kSlack);
			b = out.p;
			lim = out.cap - kSlack;
		}
	};
	auto wrap_from = [&](size_t base, size_t from) {
		out.pos = o;
		wrap(pending, base, out, from);
		b = out.p;
		o = out.pos;
		lim = out.cap > kSlack ? out.cap - kSlack : 0;
	};

	uint32_t i = choice != NONE ? choice : top >= 2 ? pick(e, top) : 0;
	const Alt* a = &alts[groups[0].first_alt + i];
	if (a->carry != NONE) {
		replay(p, a->carry, a->start, pending);
	}
	uint32_t pos = a->start;
	uint32_t next = a->first_group;
	// The group being walked.
	uint32_t end = groups[0].end;
	uint32_t after = groups[0].after;
	uint32_t wrap_base = 0;
	uint32_t pending_base = 0;
	size_t out_start = o;
	const unsigned char* k = symbols;
	for (;;) {
		unsigned char kind = pos == end ? static_cast<unsigned char>(BAR) : k[static_cast<unsigned char>(p[pos])];
		switch (kind) {
			case SYMBOL: {
				const Table& t = tables[static_cast<unsigned char>(p[pos])];
				uint32_t j = t.n >= 2 ? pick(e, t.n) : 0;
				uint32_t w = t.begins[j];
				size_t n = t.begins[j + 1] - w;
				room(n);
				size_t from = o;
				copy_blocks(b + o, t.text + w, n);
				o += n;
				if (pending.size() != pending_base) {
					wrap_from(pending_base, from);
				}
				++pos;
				break;
			}
			case CHAR: {
				size_t from = o;
				uint32_t q = pos + 1;
				if (pending.size() == pending_base) {
					// A run of characters, written at once.
					while (q < end && k[static_cast<unsigned char>(p[q])] == CHAR) {
						++q;
					}
				}
				size_t n = q - pos;
				room(n);
				if (pos + n + kSlack <= v.readable) {
					copy_blocks(b + o, p + pos, n);
				} else {
					std::memcpy(b + o, p + pos, n);
				}
				o += n;
				if (pending.size() != pending_base) {
					wrap_from(pending_base, from);
				}
				pos = q;
				break;
			}
			case CAPITALIZE:
			case REVERSE:
				pending.push(kind);
				++pos;
				break;
			case OPEN_SYMBOLS:
			case OPEN_LITERAL: {
				const Group& g = groups[next];
				uint32_t n = g.alts;
				if (n == 0) {
					// An empty group: its wrappers have nothing to change.
					pending.resize(pending_base);
					pos = g.end + 1;
					next = g.after;
					break;
				}
				uint32_t j = n >= 2 ? pick(e, n) : 0;
				frames.push(Frame{end, after, wrap_base, pending_base, out_start, k});
				wrap_base = pending_base;
				pending_base = static_cast<uint32_t>(pending.size());
				out_start = o;
				k = kind == OPEN_LITERAL ? literal : symbols;
				end = g.end;
				after = g.after;
				a = &alts[g.first_alt + j];
				if (a->carry != NONE) {
					replay(p, a->carry, a->start, pending);
				}
				pos = a->start;
				next = a->first_group;
				break;
			}
			default: {
				// The alternative is complete (a bar, or its group's end): its group is too.
				pending.resize(pending_base);
				if (!frames.size()) {
					out.pos = o;
					return;
				}
				if (pending.size() != wrap_base) {
					wrap_from(wrap_base, out_start);
				}
				pos = end + 1;
				next = after;
				const Frame& f = frames.top();
				end = f.end;
				after = f.after;
				wrap_base = f.wrap_base;
				pending_base = f.pending_base;
				out_start = f.out_start;
				k = f.k;
				frames.pop();
				break;
			}
		}
	}
}


// A compiled pattern: the pattern's text and its groups and alternatives. It stands
// for the root of the original's tree: a Collapser when collapse_triples, else the
// top level (Literal("") without alternatives, a Sequence with one, a Random with
// more), and generators added to it join it as they joined that root.
class Compiled final : public Generator {
	std::string text;  // the pattern, then kSlack NULs
	std::vector<Group> groups;
	std::vector<Alt> alts;
	const Classes& cls = classes();
	const Table* tables = symbol_tables();
	size_t c = 1, lo = 0, hi = 0;
	bool collapse;
	bool ascii;  // every byte of the pattern is ASCII 1 to 127, and so is every name

	uint32_t top() const { return groups[0].alts; }
	bool choice_root() const { return !collapse && top() >= 2; }
	bool literal_root() const { return !collapse && top() == 0; }

	void run(Out& out, std::mt19937& e, uint32_t choice) const
	{
		execute(View{text.data(), text.size(), groups.data(), alts.data(), cls, tables}, out, e, choice);
	}

public:
	Compiled(const std::string& pattern, bool collapse_triples) : collapse(collapse_triples)
	{
		if (pattern.size() >= NONE) {
			throw std::length_error("Pattern too long");
		}
		Recorder<true> r;
		scan(pattern.data(), static_cast<uint32_t>(pattern.size()), r);
		groups.assign(r.groups.data(), r.groups.data() + r.groups.size());
		alts.assign(r.alts.data(), r.alts.data() + r.alts.size());
		c = r.c;
		lo = r.lo;
		hi = r.hi;
		ascii = plain(pattern.data(), pattern.size());
		text.reserve(pattern.size() + kSlack);
		text = pattern;
		text.append(kSlack, '\0');
	}

	size_t combinations() const override
	{
		if (generators.empty() || literal_root()) {
			return c;
		}
		size_t total = c;
		for (auto& g : generators) {
			total = choice_root() ? saturating_add(total, g->combinations()) : saturating_multiply(total, g->combinations());
		}
		return total;
	}

	size_t min() const override
	{
		if (generators.empty() || literal_root()) {
			return lo;
		}
		size_t m = lo;
		for (auto& g : generators) {
			m = choice_root() ? std::min(m, g->min()) : m + g->min();
		}
		return m;
	}

	size_t max() const override
	{
		if (generators.empty() || literal_root()) {
			return hi;
		}
		size_t m = hi;
		for (auto& g : generators) {
			m = choice_root() ? std::max(m, g->max()) : m + g->max();
		}
		return m;
	}

	std::string toString() const override
	{
		std::string str;
		append(str, engine());
		return str;
	}

protected:
	void append(std::string& out, std::mt19937& e) const override
	{
		char local[256];
		if (generators.empty() && hi + kSlack <= sizeof local) {
			Out o(local, sizeof local);
			o.ascii = ascii;
			run(o, e, NONE);
			if (collapse) {
				collapse_text(o, 0);
			}
			out.append(o.p, o.pos);
			return;
		}
		size_t base = out.size();
		Out o(out);
		o.reserve(std::min<size_t>(hi, 1024));
		if (generators.empty()) {
			o.ascii = ascii;
			run(o, e, NONE);
			if (collapse) {
				collapse_text(o, base);
			}
			o.finish();
			return;
		}
		if (literal_root()) {
			o.finish();
			return;
		}
		if (choice_root()) {
			size_t i = choose(e, top() + generators.size());
			if (i < top()) {
				run(o, e, static_cast<uint32_t>(i));
				o.finish();
			} else {
				o.finish();
				append_output(*generators[i - top()], out, e);
			}
			return;
		}
		run(o, e, NONE);
		o.finish();
		append_generators(out, e);
		if (collapse) {
			Out after(out);
			collapse_text(after, base);
			after.finish();
		}
	}
};

}  // namespace


Generator::Generator()
{
}


Generator::Generator(std::vector<std::unique_ptr<Generator>>&& generators_) :
	generators(std::move(generators_))
{
}


Generator::Generator(const std::string& pattern, bool collapse_triples)
{
	add(make_unique<Compiled>(pattern, collapse_triples));
}


size_t Generator::combinations() const
{
	size_t total = 1;
	for (auto& g : generators) {
		total = saturating_multiply(total, g->combinations());
	}
	return total;
}


size_t Generator::min() const
{
	size_t final = 0;
	for (auto& g : generators) {
		final += g->min();
	}
	return final;
}


size_t Generator::max() const
{
	size_t final = 0;
	for (auto& g : generators) {
		final += g->max();
	}
	return final;
}


// Every class of the library produces its output with append(), into one string. The
// typeid() test sends a class derived from one of them to its own toString().
void Generator::append(std::string& out, std::mt19937& e) const
{
	if (typeid(*this) == typeid(Generator)) {
		append_generators(out, e);
	} else {
		out += toString();
	}
}


void Generator::append_generators(std::string& out, std::mt19937& e) const
{
	for (auto& g : generators) {
		g->append(out, e);
	}
}


std::string Generator::toString() const {
	std::string str;
	append_generators(str, engine());
	return str;
}


std::string Generator::generate(std::mt19937& e) const
{
	std::string str;
	append(str, e);
	return str;
}


void Generator::generate(std::string& name) const
{
	name.clear();
	append(name, engine());
}


void Generator::generate(std::string& name, std::mt19937& e) const
{
	name.clear();
	append(name, e);
}


void Generator::add(std::unique_ptr<Generator>&& g)
{
	generators.push_back(std::move(g));
}


Random::Random()
{
}

Random::Random(std::vector<std::unique_ptr<Generator>>&& generators_) :
	Generator(std::move(generators_))
{
}

size_t Random::combinations() const
{
	size_t total = 0;
	for (auto& g : generators) {
		total = saturating_add(total, g->combinations());
	}
	return total ? total : 1;
}

size_t Random::min() const
{
	size_t final = -1;
	for (auto& g : generators) {
		size_t current = g->min();
		if (current < final) {
			final = current;
		}
	}
	return final;
}

size_t Random::max() const
{
	size_t final = 0;
	for (auto& g : generators) {
		size_t current = g->max();
		if (current > final) {
			final = current;
		}
	}
	return final;
}


std::string Random::toString() const
{
	std::string str;
	append_own(str, engine());
	return str;
}


void Random::append(std::string& out, std::mt19937& e) const
{
	if (typeid(*this) == typeid(Random)) {
		append_own(out, e);
	} else {
		out += toString();
	}
}


void Random::append_own(std::string& out, std::mt19937& e) const
{
	if (!generators.size()) {
		return;
	}
	append_output(*generators[choose(e, generators.size())], out, e);
}


Sequence::Sequence()
{
}

Sequence::Sequence(std::vector<std::unique_ptr<Generator>>&& generators_) :
	Generator(std::move(generators_))
{
}

void Sequence::append(std::string& out, std::mt19937& e) const
{
	if (typeid(*this) == typeid(Sequence)) {
		append_generators(out, e);
	} else {
		out += toString();
	}
}

Literal::Literal(const std::string &value_) :
	value(value_)
{
}

size_t Literal::combinations() const
{
	return 1;
}

size_t Literal::min() const
{
	return value.size();
}

size_t Literal::max() const
{
	return value.size();
}

std::string Literal::toString() const
{
	return value;
}

void Literal::append(std::string& out, std::mt19937&) const
{
	if (typeid(*this) == typeid(Literal)) {
		out += value;
	} else {
		out += toString();
	}
}

Reverser::Reverser(std::unique_ptr<Generator>&& g)
{
	add(std::move(g));
}


std::string Reverser::toString() const
{
	std::string str;
	append_own(str, engine());
	return str;
}

void Reverser::append(std::string& out, std::mt19937& e) const
{
	if (typeid(*this) == typeid(Reverser)) {
		append_own(out, e);
	} else {
		out += toString();
	}
}

void Reverser::append_own(std::string& out, std::mt19937& e) const
{
	size_t from = out.size();
	append_generators(out, e);
	Out o(out);
	reverse_text(o, from);
	o.finish();
}

Capitalizer::Capitalizer(std::unique_ptr<Generator>&& g)
{
	add(std::move(g));
}

std::string Capitalizer::toString() const
{
	std::string str;
	append_own(str, engine());
	return str;
}

void Capitalizer::append(std::string& out, std::mt19937& e) const
{
	if (typeid(*this) == typeid(Capitalizer)) {
		append_own(out, e);
	} else {
		out += toString();
	}
}

void Capitalizer::append_own(std::string& out, std::mt19937& e) const
{
	size_t from = out.size();
	append_generators(out, e);
	Out o(out);
	capitalize_text(o, from);
	o.finish();
}


Collapser::Collapser(std::unique_ptr<Generator>&& g)
{
	add(std::move(g));
}

std::string Collapser::toString() const
{
	std::string str;
	append_own(str, engine());
	return str;
}

void Collapser::append(std::string& out, std::mt19937& e) const
{
	if (typeid(*this) == typeid(Collapser)) {
		append_own(out, e);
	} else {
		out += toString();
	}
}

void Collapser::append_own(std::string& out, std::mt19937& e) const
{
	size_t from = out.size();
	append_generators(out, e);
	Out o(out);
	collapse_text(o, from);
	o.finish();
}


void NameGen::generate(std::string& name, const char* pattern, size_t length, std::mt19937& e, bool collapse_triples)
{
	if (length >= NONE) {
		throw std::length_error("Pattern too long");
	}
	const Classes& cls = classes();
	char local[256];
	Out out(local, sizeof local);
	out.ascii = plain(pattern, length);
	// A pattern without groups or bars is its top level's only alternative (or none, if
	// it has only wrappers): what the recorder would record, without recording.
	size_t i = 0;
	bool component = false;
	for (; i < length; ++i) {
		unsigned char k = cls.symbols[static_cast<unsigned char>(pattern[i])];
		if (k >= BAR) {
			break;
		}
		component |= k <= SYMBOL;
	}
	if (i == length) {
		Group top{static_cast<uint32_t>(length), 1, component ? 1u : 0u, 0};
		Alt only{0, NONE, 1};
		execute(View{pattern, length, &top, &only, cls, symbol_tables()}, out, e, NONE);
	} else {
		Recorder<false> r;
		scan(pattern, static_cast<uint32_t>(length), r);
		execute(View{pattern, length, r.groups.data(), r.alts.data(), cls, symbol_tables()}, out, e, NONE);
	}
	if (collapse_triples) {
		collapse_text(out, 0);
	}
	name.assign(out.p, out.pos);
}


std::string NameGen::generate(const char* pattern, std::mt19937& e, bool collapse_triples)
{
	std::string name;
	generate(name, pattern, std::strlen(pattern), e, collapse_triples);
	return name;
}


std::string NameGen::generate(const std::string& pattern, std::mt19937& e, bool collapse_triples)
{
	std::string name;
	generate(name, pattern.data(), pattern.size(), e, collapse_triples);
	return name;
}


std::wstring towstring(const std::string & s)
{
	const char *cs = s.c_str();
	const size_t wn = std::mbsrtowcs(nullptr, &cs, 0, nullptr);

	if (wn == static_cast<size_t>(-1)) {
		return L"";
	}

	std::vector<wchar_t> buf(wn);
	cs = s.c_str();
	const size_t wn_again = std::mbsrtowcs(buf.data(), &cs, wn, nullptr);

	if (wn_again == static_cast<size_t>(-1)) {
		return L"";
	}

	return std::wstring(buf.data(), wn);
}

std::string tostring(const std::wstring & s)
{
	const wchar_t *cs = s.c_str();
	const size_t wn = std::wcsrtombs(nullptr, &cs, 0, nullptr);

	if (wn == static_cast<size_t>(-1)) {
		return "";
	}

	std::vector<char> buf(wn);
	const size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn, nullptr);

	if (wn_again == static_cast<size_t>(-1)) {
		return "";
	}

	return std::string(buf.data(), wn);
}
