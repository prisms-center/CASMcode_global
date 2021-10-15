#include "casm/casm_io/Log.hh"

#include "casm/external/MersenneTwister/MersenneTwister.h"
#include "casm/misc/string_algorithm.hh"

namespace CASM {

/// \class Log
///
/// \brief Formatted logging
///
/// The Log class manages an output stream and formatting options, such as
/// verbosity level, indent level, and paragraph width and justification, to
/// format output.
///
/// Features include:
/// - Setting indents
/// - Hiding / showing output from different sections of code based on a
///   verbosity level setting
/// - Write justified paragraphs
/// - Write "verbatim" (not justified) indented text
///
/// ### Controlling indentation
///
/// Example: Set indent level directly (`Log::set_indent_level`)
/// \code
/// // default indent is 2 spaces per indent level, 0 initial spaces indent
/// Log log;
/// log.indent() << "line 1" << std::endl;
/// log.set_indent_level(1);
/// log.indent() << "line 2" << std::endl;
/// log.set_indent_level(0);
/// log.indent() << "line 3" << std::endl;
///
/// std::string expected =
///   "line 1\n"
///   "  line 2\n"
///   "line 3\n";
/// \endcode
///
/// Example: Increment / decrement indent level (`Log::increase_indent` /
///     `Log::decrease_indent`)
/// \code
/// Log log;
/// log.indent() << "line 1" << std::endl;
/// log.increase_indent();
/// log.indent() << "line 2" << std::endl;
/// log.decrease_indent();
/// log.indent() << "line 3" << std::endl;
///
/// std::string expected =
///   "line 1\n"
///   "  line 2\n"
///   "line 3\n";
/// \endcode
///
/// Example: Set indent size (`Log::set_indent_space`)
/// \code
/// Log log;
/// log.set_indent_space(4);
/// log.indent() << "line 1" << std::endl;
/// log.increase_indent();
/// log.indent() << "line 2" << std::endl;
/// log.decrease_indent(0);
/// log.indent() << "line 3" << std::endl;
///
/// std::string expected =
///   "line 1\n"
///   "    line 2\n"
///   "line 3\n";
/// \endcode
///
/// Example: Set an initial indent size (`Log::initial_indent_space`)
/// \code
/// Log log;
/// log.set_indent_space(4);
/// log.initial_indent_space(2);
/// log.indent() << "line 1" << std::endl;
/// log.increase_indent();
/// log.indent() << "line 2" << std::endl;
/// log.decrease_indent(0);
/// log.indent() << "line 3" << std::endl;
/// std::string expected =
///   "  line 1\n"
///   "      line 2\n"
///   "  line 3\n";
/// \endcode
///
/// ### Writing paragraphs (set indentation, justification, and width)
///
/// Example: Specify paragraph width (`Log::set_width`, `Log::paragraph`)
/// \code
/// Log log;
/// log.set_width(10);
/// log.paragraph("The quick brown fox jumps over the lazy dog.");
///
/// std::string expected =
///   "The quick\n"
///   "brown fox\n"
///   "jumps over\n"
///   "the lazy\n"
///   "dog.\n";
/// \endcode
///
/// Example: Specify paragraph indent & width (`Log::set_width`,
///     `Log::paragraph`)
/// \code
/// Log log;
/// log.set_width(20);
/// log.increase_indent();
/// log.paragraph("The quick brown fox jumps over the lazy dog.");
///
/// std::string expected =
///   "  The quick brown\n"
///   "  fox jumps over the\n"
///   "  lazy dog.\n";
/// \endcode
///
/// Example: Full justified paragraph (`Log::set_justification`)
/// \code
/// Log log;
/// log.set_width(20);
/// log.increase_indent();
/// log.set_justification(JustificationType::Full);
/// log.paragraph("The quick brown fox jumps over the lazy dog.");
///
/// std::string expected =
///   "  The   quick  brown\n"
///   "  fox jumps over the\n"
///   "  lazy dog.\n";
/// \endcode
///
/// Example: Right justified paragraph (`Log::set_justification`)
/// \code
/// Log log;
/// log.set_justification(JustificationType::Right);
/// log.set_width(10);
/// log.paragraph("The quick brown fox jumps over the lazy dog.");
///
/// std::string expected =
///   " The quick\n"
///   " brown fox\n"
///   "jumps over\n"
///   "  the lazy\n"
///   "      dog.\n";
/// \endcode
///
/// Example: Center justified paragraph (`Log::set_justification`)
/// \code
/// Log log;
/// log.set_justification(JustificationType::Center);
/// log.set_width(10);
/// log.paragraph("The quick brown fox jumps over the lazy dog.");
///
/// std::string expected =
///   "   The quick brown  \n"
///   "  fox jumps over the\n"
///   "      lazy dog.     \n";
/// \endcode
///
/// ### Controlling output verbosity
///
/// A Log tracks "sections" of code, which each have a required verbosity
/// level. Separately, a current verbosity level may be set, such as by user
/// input, or when debugging test code. Then, as sections of the code are
/// encountered, writing to the Log only occurs if the current verbosity level
/// is greater than or equal to the required verbosity level of the current
/// section.
///
/// Log includes the following constants for named verbosity levels:
/// - static const int Log::none = 0;
/// - static const int Log::quiet = 5;
/// - static const int Log::standard = 10;
/// - static const int Log::verbose = 20;
/// - static const int Log::debug = 100;
///
/// Example: Set verbosity level and required verbosity (`Log::set_verbosity`,
///     `Log::begin_section`, `Log::end_section`)
///
/// \code
/// // default required verbosity == Log::standard
/// Log log;
///
/// // set verbosity level (int in range [0, 100])
/// int verbosity_level = Log::verbose;
/// log.set_verbosity(verbosity_level);
///
/// // write line 1 if verbosity level >= Log::standard
/// log << "line 1: usually print this" << std::endl;
/// log.end_section();
///
/// // write line 2 if verbosity level >= Log::none
/// log.begin_section<Log::none>();
/// log << "line 2: always print this" << std::endl;
/// log.end_section();
///
/// // write line 3 if verbosity level >= Log::quiet
/// log.begin_section<Log::quiet>();
/// log << "line 3: print this, even in quiet mode" << std::endl;
/// log.end_section();
///
/// // write line 4 if verbosity level >= Log::standard
/// log.begin_section<Log::standard>();
/// log << "line 4: usually print this" << std::endl;
/// log.end_section();
///
/// // write line 5 if verbosity level >= Log::verbose
/// log.begin_section<Log::verbose>();
/// log << "line 5: print this in verbose or debug mode" << std::endl;
/// log.end_section();
///
/// // write line 6 if verbosity level >= Log::debug
/// log.begin_section<Log::debug>();
/// log << "line 6: only print this in debug mode" << std::endl;
/// log.end_section();
/// \endcode
///
/// Example: Creating subsections
///
/// Sections are stored like a "stack" so that new sections can be treated as
/// subsections:
/// - `Log::begin_section()` pushes a new section on a stack
/// - `Log::end_section()` pops the last section off the stack
///
/// \code
/// Log log; // default to "standard" verbosity
/// log.set_verbosity(Log::standard);
/// log << "line 1: print this at >= standard verbosity" << std::endl;
///
/// log.begin_section<Log::verbose>();
/// log << "line 2: print this at >= verbose verbosity" << std::endl;
///
/// log.begin_section<Log::standard>();
/// log << "line 3: print this at >= standard verbosity" << std::endl;
/// log.end_section(); // end "standard" section
///
/// log << "line 4: print this at >= verbose verbosity" << std::endl;
/// log.end_section(); // end "verbose" section
///
/// log << "line 5: print this at >= standard verbosity" << std::endl;
///
/// // Expected output:
/// // line 1: print this at >= standard verbosity
/// // line 3: print this at >= standard verbosity
/// // line 5: print this at >= standard verbosity
/// \endcode
///
/// ### Standard sections
///
/// To cut down on excessive `end_section()` and `begin_section()` usages, Log
/// methods that combine the following:
/// - end the current section
/// - begin a new section
/// - write an indented, formatted, header for the section:
///   - format: "<indent spaces>-- <type>: <what> -- \n"
///
/// Example: Standard section header (`Log::calculate`)
/// \code
/// Log log;
/// log.increase_indent();
/// log.calculate<Log::standard>("Something");
/// log.indent() << "Using ..." << std::endl;
///
/// std::string expected =
///   "  -- Calculate: Something -- \n"
///   "  Using ...\n";
/// \endcode
///
/// Standard sections include: `Log::calculate`, `Log::construct`,
///   `Log::generate`, `Log::error`, `Log::warning`, etc.
///
/// The `Log::custom` method can be used for custom section headers.
///
/// Example: Custom section header (`Log::custom`)
/// `Log::calculate` usage:
/// \code
/// Log log;
/// log.increase_indent();
/// log.custom<Log::standard>("Section heading");
/// log.indent() << "Using ..." << std::endl;
///
/// std::string expected =
///   "  -- Section heading -- \n"
///   "  Using ...\n";
/// \endcode
///
/// ### Run time calculation
///
/// Log can also be used to track execution time using:
/// - `Log::restart_clock`: Restart internal clock
/// - `Log::time_s()`: Time in seconds since Log construction or
///   `Log::restart_clock`
/// - `Log::show_clock()`: Show `Log::time_s()` as a part of section headings.
/// - `Log::hide_clock()`: Do not show time as a part of section headings.
/// - `Log::begin_lap()`: Begin a "lap timer". Does not restart the primary
///   internal clock.
/// - `Log::lap_time()`: Time in seconds since `Log::begin_lap()`.
///
/// Example: Timing exection with Log
/// \code
/// Log log;
/// log.show_clock();
///
/// log.calculate<Log::standard>("Something");
/// log.begin_lap();
/// log.indent() << do_something(100) << std::endl;
/// log.indent() << "DONE... took " << log.lap_time() << " s" << std::endl;
/// log << std::endl;
///
/// // Expected output:
/// // -- Calculate: Something -- Time: 1.2498e-05 (s)
/// // 21424814486779
/// // DONE... took 3.4613e-05 s
/// \endcode
///
/// \ingroup LogGroup

const int Log::none;
const int Log::quiet;
const int Log::standard;
const int Log::verbose;
const int Log::debug;

/// \brief Construct a Log
///
/// \param ostream The stream to print to
/// \param verbosity The amount to be printed
///
/// For verbosity:
/// - 0: print nothing
/// - 10: print all standard output
/// - 100: print all possible output
Log::Log(std::ostream &_ostream, int _verbosity, bool _show_clock,
         int _indent_space)
    : m_verbosity(_verbosity),
      m_show_clock(_show_clock),
      m_indent_space(_indent_space),
      m_indent_level(0),
      m_indent_spaces(0),
      m_paragraph_width(100),
      m_justification(JustificationType::Left),
      m_ostream(&_ostream) {
  restart_clock();
  begin_section();
}

void Log::restart_clock() { m_start_time = std::chrono::steady_clock::now(); }

void Log::show_clock() { m_show_clock = true; }

void Log::hide_clock() { m_show_clock = false; }

double Log::time_s() const {
  using namespace std::chrono;
  auto curr_time = steady_clock::now();
  return duration_cast<duration<double> >(curr_time - m_start_time).count();
}

void Log::begin_lap() { m_lap_start_time = std::chrono::steady_clock::now(); }

double Log::lap_time() const {
  using namespace std::chrono;
  auto curr_time = steady_clock::now();
  return duration_cast<duration<double> >(curr_time - m_lap_start_time).count();
}

/// \brief Get current verbosity level
int Log::verbosity() const { return m_verbosity; }

/// \brief Set current verbosity level
void Log::set_verbosity(int _verbosity) { m_verbosity = _verbosity; }

/// \brief Reset underlying stream
void Log::reset(std::ostream &_ostream) { m_ostream = &_ostream; }

/// \brief Choose c random unique numbers in [0,n)
std::vector<int> rand_unique(int n, int c, MTRand &mtrand) {
  std::vector<int> index;
  for (int i = 0; i < n; ++i) {
    index.push_back(i);
  }
  using std::swap;
  int choice;
  int s = index.size();
  std::vector<int> res;
  for (int i = 0; i < c; ++i) {
    choice = mtrand.randInt(s - 1);
    res.push_back(index[choice]);
    swap(index[s - 1], index[choice]);
    s--;
  }

  return res;
}

void Log::_print_justified_line(std::vector<std::string> &line,
                                int curr_width) {
  // treat too-long case as left justified
  if (justification() == JustificationType::Left ||
      curr_width + line.size() - 1 >= width()) {
    _print_left_justified_line(line, curr_width);
  } else if (justification() == JustificationType::Right) {
    _print_right_justified_line(line, curr_width);
  } else if (justification() == JustificationType::Center) {
    _print_center_justified_line(line, curr_width);
  } else if (justification() == JustificationType::Full) {
    _print_full_justified_line(line, curr_width);
  } else {
    throw std::runtime_error("Log print justification error");
  }
}

void Log::_print_left_justified_line(std::vector<std::string> &line,
                                     int curr_width) {
  indent();
  for (int i = 0; i < line.size(); ++i) {
    if (i != 0) {
      *this << " ";
    }
    *this << line[i];
  }
  *this << std::endl;
}

void Log::_print_right_justified_line(std::vector<std::string> &line,
                                      int curr_width) {
  indent();
  std::stringstream ss;
  for (int i = 0; i < line.size(); ++i) {
    if (i != 0) {
      ss << " ";
    }
    ss << line[i];
  }
  *this << std::string(width() - indent_str().size() - ss.str().size(), ' ')
        << ss.str() << std::endl;
}

void Log::_print_center_justified_line(std::vector<std::string> &line,
                                       int curr_width) {
  indent();
  std::stringstream ss;
  for (int i = 0; i < line.size(); ++i) {
    if (i != 0) {
      ss << " ";
    }
    ss << line[i];
  }
  std::string str = ss.str();
  int fill_size = width() - indent_str().size() - str.size();
  std::string before = std::string(fill_size / 2, ' ');
  std::string after = std::string(fill_size - before.size(), ' ');
  *this << before << str << after << std::endl;
}

void Log::_print_full_justified_line(std::vector<std::string> &line,
                                     int curr_width) {
  indent();
  // add ' ' evenly as much as possible
  while (width() - curr_width >= line.size() - 1) {
    for (int i = 0; i < line.size() - 1; ++i) {
      line[i] += ' ';
      curr_width++;
    }
  }
  // add extra uneven ' ' using random number generator to choose locations
  // but seed based on curr_width to give consistent results
  MTRand mtrand(curr_width);
  std::vector<int> index =
      rand_unique(line.size() - 1, width() - curr_width, mtrand);
  for (int i = 0; i < index.size(); ++i) {
    line[i] += ' ';
  }
  // print words (which now include spaces)
  for (auto &word : line) {
    *this << word;
  }
  *this << std::endl;
}

/// \brief Print indented, justified, paragraph with line wrapping
///
/// \param text Text to write as a paragraph.
///
/// Line wrapping occurs at `Log::width()`, including the initial indent.
Log &Log::paragraph(std::string text) {
  char_separator sep(" ");
  tokenizer tok(text, sep);
  std::vector<std::string> words(tok.begin(), tok.end());

  // 'curr_width' includes indent and words, but not spaces between them
  int curr_width = indent_str().size();
  std::vector<std::string> line;
  for (int i = 0; i < words.size(); ++i) {
    if (line.size() == 0 ||
        curr_width + line.size() + words[i].size() <= width()) {
      line.push_back(words[i]);
      curr_width += words[i].size();
    } else {
      // print not-last line
      _print_justified_line(line, curr_width);

      // begin next line
      line.clear();
      line.push_back(words[i]);
      curr_width = indent_str().size() + words[i].size();
    }
  }
  // print last line
  if (justification() == JustificationType::Full) {
    _print_left_justified_line(line, curr_width);
  } else {
    _print_justified_line(line, curr_width);
  }

  return *this;
}

/// \brief Print verbatim, but with indentation (optional on first line)
///
/// \param text Text to write as a paragraph. Uses `std::getline` to get lines,
///     which are each written after adding indentation. Unlike
///     `Log::paragraph`, no justification is performed.
/// \param indent_first_line Option determines whether or not an indentation is
///     added before the first line written.
Log &Log::verbatim(std::string text, bool indent_first_line) {
  std::istringstream input;
  input.str(text);
  std::string first_line;
  if (std::getline(input, first_line)) {
    if (indent_first_line) {
      *this << indent_str();
    }
    *this << first_line << std::endl;
    for (std::string line; std::getline(input, line);) {
      indent() << line << std::endl;
    }
  }
  return *this;
}

Log::operator std::ostream &() { return ostream(); }

std::string Log::invalid_verbosity_msg(std::string s) {
  return std::string("Error: Received '") + s +
         "', expected one of 'none', 'quiet', 'standard', 'verbose', 'debug', "
         "or an int in range [0, 100]";
}

/// \brief Read verbosity level from a string
///
/// \returns result, a pair of bool,int
///          result.first == true if successfully read,
///          and result.second is the verbosity level
///
std::pair<bool, int> Log::verbosity_level(std::string s) {
  auto is_int = [](std::string s) {
    int val;
    if (s.empty() || !isdigit(s[0])) {
      return std::make_pair(false, val);
    }
    char *p;
    val = strtol(s.c_str(), &p, 10);
    return std::make_pair(*p == 0 && val >= 0 && val <= 100, val);
  };

  auto res = is_int(s);
  if (res.first) {
    return res;
  } else if (s == "none") {
    return std::make_pair(true, 0);
  } else if (s == "quiet") {
    return std::make_pair(true, 5);
  } else if (s == "standard") {
    return std::make_pair(true, 10);
  } else if (s == "verbose") {
    return std::make_pair(true, 20);
  } else if (s == "debug") {
    return std::make_pair(true, 100);
  } else {
    return std::make_pair(false, 0);
  }
};

/// \brief If true, indicates the current verbosity level is greater than or
///     equal to the current required verbosity
bool Log::print() const { return _print(); }

void Log::_add_time() {
  if (m_show_clock) {
    ostream() << "Time: " << time_s() << " (s)";
  }
}

/// \brief If true, indicates the current verbosity level is greater than or
///     equal to the current required verbosity
bool Log::_print() const { return m_print; }

/// \brief Write to Log, if required verbosity level is satisfied
///
/// If the Log's current verbosity level exceeds the Log's current required
/// verbosity level, then the write occurs; otherwise the write does not occur.
///
/// \relates Log
Log &operator<<(Log &log, std::ostream &(*fptr)(std::ostream &)) {
  if (log._print()) {
    fptr(static_cast<std::ostream &>(log));
  }
  return log;
}

FixedLog::FixedLog(std::ostream &_ostream) : Log(_ostream) {}

}  // namespace CASM
