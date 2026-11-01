/**
 *
 * @file A fantasy name generator library.
 * @version 1.0.1
 * @license Public Domain
 * @author German M. Bravo (Kronuz)
 *
 */

#include "namegen.h"

#include <algorithm>  // for move, reverse
#include <chrono>     // for rng seed
#include <cwchar>     // for size_t, mbsrtowcs, wcsrtombs
#include <cwctype>    // for towupper
#include <functional> // for hash
#include <iterator>   // for iterator_traits
#include <memory>     // for make_unique
#include <random>     // for mt19937, uniform_real_distribution
#include <stdexcept>  // for invalid_argument, out_of_range
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


// A choice among n alternatives, each with probability 1/n, from two outputs of the
// engine: u = (x1 + x2 * 2^32) / 2^64, the value std::generate_canonical<double> takes
// from a 32-bit engine in libc++, libstdc++ and the MSVC STL, then floor(u * n). The sum
// can round up to 2^64, so u can be 1.0; that draw takes the last alternative.
template <typename Engine>
static size_t choose(Engine& e, size_t n)
{
	double low = static_cast<double>(e());
	double high = static_cast<double>(e());
	double u = (low + high * 4294967296.0) / 18446744073709551616.0;
	size_t i = static_cast<size_t>(u * static_cast<double>(n));
	return i < n ? i : n - 1;
}


// A draw whose choice is known (one alternative): the engine moves on as it would.
static void skip(std::mt19937& e, size_t draws)
{
	e.discard(2 * static_cast<unsigned long long>(draws));
}


namespace {

// The compiler's nodes for what the original built as a Random of one Literal per
// alternative: a symbol (one node over the symbol's table, no copy of its strings)
// and a run of literal characters (one node for the run, one draw per character as
// the original's one-alternative Random made). Nothing outside the compiled tree
// reaches them; they count and draw as the nodes they stand for.
struct SymbolTable {
	const std::vector<std::string>* words = nullptr;
	size_t min = 0;
	size_t max = 0;
};


class Symbol final : public Generator {
	const SymbolTable& table;

public:
	explicit Symbol(const SymbolTable& table_) : table(table_) {}

	size_t combinations() const override { return table.words->size(); }
	size_t min() const override { return table.min; }
	size_t max() const override { return table.max; }

	std::string toString() const override
	{
		std::string str;
		append(str, engine());
		return str;
	}

protected:
	void append(std::string& out, std::mt19937& e) const override
	{
		out += (*table.words)[choose(e, table.words->size())];
	}
};


class Run final : public Generator {
	std::string text;

public:
	explicit Run(char c) : text(1, c) {}

	void push(char c) { text += c; }

	size_t combinations() const override { return 1; }
	size_t min() const override { return text.size(); }
	size_t max() const override { return text.size(); }

	std::string toString() const override
	{
		std::string str;
		append(str, engine());
		return str;
	}

protected:
	void append(std::string& out, std::mt19937& e) const override
	{
		skip(e, text.size());
		out += text;
	}
};


// The symbol tables by character, built once from SymbolMap().
const SymbolTable* symbol_table(char c)
{
	static const auto* const tables = [] {
		auto* t = new SymbolTable[256];
		for (const auto& entry : Generator::SymbolMap()) {
			if (entry.first.size() == 1 && !entry.second.empty()) {
				auto& table = t[static_cast<unsigned char>(entry.first[0])];
				table.words = &entry.second;
				table.min = static_cast<size_t>(-1);
				for (const auto& w : entry.second) {
					table.min = w.size() < table.min ? w.size() : table.min;
					table.max = w.size() > table.max ? w.size() : table.max;
				}
			}
		}
		return t;
	}();
	const SymbolTable* table = &tables[static_cast<unsigned char>(c)];
	return table->words ? table : nullptr;
}

}  // namespace


Generator::Generator()
{
}


Generator::Generator(std::vector<std::unique_ptr<Generator>>&& generators_) :
	generators(std::move(generators_))
{
}


// Counts that do not fit in a size_t saturate at its maximum rather than wrap.
static size_t saturating_add(size_t a, size_t b)
{
	return a > static_cast<size_t>(-1) - b ? static_cast<size_t>(-1) : a + b;
}


static size_t saturating_multiply(size_t a, size_t b)
{
	return a != 0 && b > static_cast<size_t>(-1) / a ? static_cast<size_t>(-1) : a * b;
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

// Reverser, Capitalizer and Collapser work on characters: they decode the bytes with
// the C library's multibyte functions (in the current locale), transform, and encode
// back; bytes the locale does not decode give "", and a NUL ends the text. Bytes 1 to
// 127 are ASCII, which decodes to itself in every locale the C libraries ship (each
// is a superset of ASCII), so a text of only those bytes is transformed in place, with
// no conversion and no allocation, and gives what the conversion gives.
static bool plain(const std::string& s, size_t from)
{
	for (size_t i = from; i < s.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(s[i]);
		if (c == 0 || c > 0x7f) {
			return false;
		}
	}
	return true;
}


// Replaces out[from, end) by f(that text, decoded), encoded back.
template <typename F>
static void transform_decoded(std::string& out, size_t from, F f)
{
	std::wstring str = towstring(out.substr(from));
	f(str);
	out.resize(from);
	out += tostring(str);
}


void Reverser::append_own(std::string& out, std::mt19937& e) const
{
	size_t from = out.size();
	append_generators(out, e);
	if (plain(out, from)) {
		std::reverse(out.begin() + from, out.end());
		return;
	}
	transform_decoded(out, from, [](std::wstring& str) {
		std::reverse(str.begin(), str.end());
	});
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
	if (plain(out, from)) {
		if (from == out.size()) {
			return;
		}
		// towupper() can leave ASCII (a Turkish locale maps i to U+0130): then the
		// encoding takes over.
		std::wint_t upper = std::towupper(static_cast<unsigned char>(out[from]));
		if (upper > 0 && upper <= 0x7f) {
			out[from] = static_cast<char>(upper);
			return;
		}
	}
	transform_decoded(out, from, [](std::wstring& str) {
		if (!str.empty()) {
			str[0] = std::towupper(str[0]);
		}
	});
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

// Keeps at most one of a run of a, h, i, j, q, u, v, w, x or y and at most two of a
// run of anything else, writing over [first, last); returns the new end.
template <typename It>
static It collapse(It first, It last)
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


void Collapser::append_own(std::string& out, std::mt19937& e) const
{
	size_t from = out.size();
	append_generators(out, e);
	if (plain(out, from)) {
		out.erase(collapse(out.begin() + from, out.end()), out.end());
		return;
	}
	transform_decoded(out, from, [](std::wstring& str) {
		str.erase(collapse(str.begin(), str.end()), str.end());
	});
}


Generator::Generator(const std::string &pattern, bool collapse_triples) {
	std::unique_ptr<Generator> last;

	std::stack<std::unique_ptr<Group>> stack;
	std::unique_ptr<Group> top = make_unique<GroupSymbol>();

	for (auto c : pattern) {
		switch (c) {
			case '<':
				stack.push(std::move(top));
				top = make_unique<GroupSymbol>();
				break;
			case '(':
				stack.push(std::move(top));
				top = make_unique<GroupLiteral>();
				break;
			case '>':
			case ')':
				if (stack.size() == 0) {
					throw std::invalid_argument("Unbalanced brackets");
				} else if (c == '>' && top->type != group_types::symbol) {
					throw std::invalid_argument("Unexpected '>' in pattern");
				} else if (c == ')' && top->type != group_types::literal) {
					throw std::invalid_argument("Unexpected ')' in pattern");
				}
                                last = top->produce();
				top = std::move(stack.top());
				stack.pop();
				top->add(std::move(last));
				break;
			case '|':
				top->split();
				break;
			case '!':
				if (top->type == group_types::symbol) {
					top->wrap(wrappers::capitalizer);
				} else {
					top->add(c);
				}
				break;
			case '~':
				if (top->type == group_types::symbol) {
					top->wrap(wrappers::reverser);
				} else {
					top->add(c);
				}
				break;
			default:
				top->add(c);
				break;
		}
	}

	if (stack.size() != 0) {
		throw std::invalid_argument("Missing closing bracket");
	}

        std::unique_ptr<Generator> g = top->produce();
	if (collapse_triples) {
		g = make_unique<Collapser>(std::move(g));
	}
	add(std::move(g));
}


Generator::Group::Group(group_types_t type_) :
	type(type_)
{
}

void Generator::Group::add(std::unique_ptr<Generator>&& g)
{
	while (!wrappers.empty()) {
		switch (wrappers.top()) {
			case reverser:
				g = make_unique<Reverser>(std::move(g));
				break;
			case capitalizer:
				g = make_unique<Capitalizer>(std::move(g));
				break;
		}
		wrappers.pop();
	}
	if (set.size() == 0) {
		set.push_back(make_unique<Sequence>());
	}
	set.back()->add(std::move(g));
	run = nullptr;
}

void Generator::Group::add(char c)
{
	// A character after a character joins its run, unless a wrapper waits for it.
	if (run && wrappers.empty()) {
		static_cast<Run*>(run)->push(c);
		return;
	}
	std::unique_ptr<Generator> g = make_unique<Run>(c);
	Generator* added = wrappers.empty() ? g.get() : nullptr;
	Group::add(std::move(g));
	run = added;
}

std::unique_ptr<Generator> Generator::Group::produce()
{
	switch (set.size()) {
		case 0:
			return make_unique<Literal>("");
		case 1:
			return std::move(*set.begin());
		default:
			return make_unique<Random>(std::move(set));
	}
}

void Generator::Group::split()
{
	if (set.size() == 0) {
		set.push_back(make_unique<Sequence>());
	}
	set.push_back(make_unique<Sequence>());
	run = nullptr;
}

void Generator::Group::wrap(wrappers_t type)
{
	wrappers.push(type);
}

Generator::GroupSymbol::GroupSymbol() :
	Group(group_types::symbol)
{
}

void Generator::GroupSymbol::add(char c)
{
	if (const SymbolTable* table = symbol_table(c)) {
		Group::add(make_unique<Symbol>(*table));
	} else {
		Group::add(c);
	}
}

Generator::GroupLiteral::GroupLiteral() :
	Group(group_types::literal)
{
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
