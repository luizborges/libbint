/*******************************************************************************/
#ifndef __BORGES_BINT_HPP__
#define __BORGES_BINT_HPP__

#include <iostream>
#include <string>
#include <sstream> // for ostringstream
#include <vector>
#include <tuple>
#include <chrono>
#include <cmath>
#include <ctype.h> 
#include <limits>       // std::numeric_limits
#include <cstdint>
#include <map>
#include <exception>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h> // abi::__cxa_demangle() - GCC only
#endif

///////////////////////////////////////////////////////////////////////////////////////
// project goal
///////////////////////////////////////////////////////////////////////////////////////
// Create a big int string class
// depois mudar isso, mas inicialmente, fazer a representação interna do número na base 256 e depois mudar para uma base
// mais elevada

///////////////////////////////////////////////////////////////////////////////////////
// project status - TODO
///////////////////////////////////////////////////////////////////////////////////////
// implementar a classe de erro criada no padrão - libutilhpp
// fazer o stacktrace da classe bint
// implementar uma versão de representação do bint 1 e -1 otimizados
// implementar um algoritmo para sqrt()
// implementar um algoritmo para root(N)
// implementar um novo algoritmo otimizado para transformar para str(). - o atual é lento
// implementar as funções min(), max(), minr(), maxr()
// implementar uma nova classe para representar o vector - abastrair isso da classe.
// implementar um algoritmo swap() entre dois bint
// implementar uma verificação nos templates para syntax suggar, e verificar o tipo char[N] -> literals string
// implementar um bint na base uint32
// implementar uma versão thread_safe
// implementar uma classe responsável por fazer um bulk de vector - otimização
// implementar uma versão para tratar os casos dos shifts maiores que long long
// implementar uma versão dos shifts <<, >>, shift_left, shift_right para big_int como parâmetro, ou seja, o número de bits a ser deslocado ser maior que o suportado por long long.
// implementar uma versão para tratar os casos de algoritmos que utilizam número de bits internamente -> pois se o número de bits for maior que long long, o algoritmo falha, algoritmos de divisão (div2, div3 e div4)
// implementar uma versão otimizada do pow(long long) em que chama o pow(bint&), pois pode acontecer do pow(long long) estourar o limite long long, devido a questões internas do algoritmo.

/////////////////////////////////////////////////////////////////////

// eu implementei parte do Str_Int que é uma biblioteca que representa um número inteiro na base 10 por meio de uma string.
// essa biblioteca é necessária para transformar uma string em um número big_int em uma base superior e facilitar a computação.
// essa biblioteca também pode ser usada para fazer operações matemáticas com números representados em string, como uma classe big_int mais simples.
// atualmente está a classe bint está com os operadores de comparação já feitos; ela já transforma um número, int, long long, string para bint(base 256); e ela já transforma um número bint(base 256) para uma string(base 10), por meio da função bint::str()
// deve-se agora -> quero fazer as seguintes coisas na classe: implementar uma classe para tratar o vetor de forma transparente que armazena a representação do número big_int -> isto devido que em termos de eficiência (libgmp) o vetor que representa um número nunca diminui de tamanho, apenas cresce, e tratar o vetor por meio de um size e um virtual_size é muito ruim, prone error, bagunça o código, etc...
// implementar uma classe big_int(base 2^32 -> tamanho int) para melhorar a performance, principalmente a questão de espaço e permitir a representação de números maiores.
// implementar um classe de erro, para tratar tudo.
// implementar as operações aritiméticas na classe Str_Int ????? -> dúvida

//possíveis nomes para a biblioteca: intb bint bigint big_int bint Bigint Big_int BigInt bigInt BIGINT BIG_INT

/**
 * @import desings:
 * 1) o vector que representa o número, é especificado do dígito menos significativo para o mais significativo.
 * Ou seja, o número: 256 (base 10), que é igual ao número 10 (base 256), é representado pelo array _number_ da seguinte forma:
 * _number_[0] = 0; _number_[1] = 1;
 * O número 356 (base 10), é representado no _number_, se este estiver usando a base 10, na seguinte forma:
 * _number_[0] = 6; _number_[1] = 5; _number_[2] = 3;
 * 
 * A escolha se deveu, pois tal formato facilita as operações de soma e multiplicação.
 */

///////////////////////////////////////////////////////////////////////////////////////
// show and sout function
///////////////////////////////////////////////////////////////////////////////////////
/**
 * As funções abaixo foram adicionadas para o casa de não se usar a biblioteca libutilpp.
 * Até conformar com ela, para a classe bint funcionar corretamente.
 */
#ifndef UTILPP_H

std::string args_to_str(const std::ostringstream &os) {
    return os.str();
}

template<typename T, typename ... Args>
std::string args_to_str(std::ostringstream &os, const T val,  const Args ... args) {
    if constexpr(std::is_pointer<T>::value)  {
		if(val == nullptr || val == NULL) {
			os << "";
		} else {
			os << val;
		}
	}
	else if constexpr(std::is_same<T, bool>::value) {
	    if(val == true) {
	        os << "true";
	    } else {
	        os << "false";
	    }
	} else {
		os << val;
	}
    
    return args_to_str(os, args ...);
}

template<typename ... Args>
void show(const Args ... args) {    
    std::ostringstream os;    
    std::cout << args_to_str(os, args ...);
}

template<typename ... Args>
std::string sout(const Args ... args) {
	std::ostringstream os;
    return args_to_str(os, args ...);
}

#if defined(__GNUC__) || defined(__clang__)
#define binterror(...) \
    bint_error(sout(__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__);

#elif defined(_MSC_VER) // compilador da microsoft
#define binterror(...) \
    bint_error(sout(__VA_ARGS__), __FILE__, __FUNCSIG__, __LINE__);

#else // qualquer outro compilador
#define binterror(...) \
    bint_error(sout(__VA_ARGS__), __FILE__, __func__, __LINE__);
#endif

#endif // UTILPP_H

///////////////////////////////////////////////////////////////////////////////////////
// project class
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .hpp file
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .hpp file
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .cpp file
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// enf of class Str_Int
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// Error class
///////////////////////////////////////////////////////////////////////////////////////
class bint_error : public std::exception {
 private:
    std::string _user_msg_;
    std::string _full_msg_;
    std::string _file_;
	std::string _func_;
	int _line_;

    std::string make_error_msg( 
        const std::string& user_msg, 
        const std::string& file,
        const std::string& func,
        const int line)
    {
        std::string str = user_msg.empty() ? "" : "ERROR - " + user_msg + "\n";
        // return str + "'" + func + "'('" + file + ",'" + std::to_string(line) + ")";
        return str + "" + func + "(" + std::to_string(line) + ",'" + file + "')\n";
    }

 public:
    // MyCustomException(char * msg) : message(msg) {}
    bint_error(const std::string& user_msg, 
        const std::string& file = "NO_FILE",
        const std::string& func = "NO_FUNCTION",
        const int line = -1) : _user_msg_(user_msg), 
        _file_(file), _func_(func), _line_(line) {
            #ifndef UTILPP_H
                _full_msg_ = make_error_msg(user_msg, file, func, line);
            #else
                _full_msg_ = borges::util::make_error_msg(user_msg, file, func, line);
            #endif
        }

    const char* what () const noexcept {
        return (_full_msg_.c_str());
    }
};

///////////////////////////////////////////////////////////////////////////////////////
// Auxiliar structs and classes and enums
///////////////////////////////////////////////////////////////////////////////////////

template<typename QUOT, typename REM>
struct Div {
    QUOT quot = 0;
    REM  rem = 0;

    // Div(){}

    // Div(const QUOT& q, const REM& r) {
    //     show("this? - r: ",r,"\n");
    //     quot = q;
    //     rem = r;
    // }

    // Div(const QUOT&& q, const REM&& r) {
    //     show("that?\n");
    //     quot = q;
    //     rem = r;
    // }

    // // Div(const QUOT q, const REM r) {
    // //     quot = q;
    // //     rem = r;
    // // }
};

enum class Relation {
    
};

// struct Bit {
//     static constexpr const std::vector<unsigned long long> pos = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648, 4294967296, 8589934592, 17179869184, 34359738368, 68719476736, 137438953472, 274877906944, 549755813888, 1099511627776, 2199023255552, 4398046511104, 8796093022208, 17592186044416, 35184372088832, 70368744177664, 140737488355328, 281474976710656, 562949953421312, 1125899906842624, 2251799813685248, 4503599627370496, 9007199254740992, 18014398509481984, 36028797018963968, 72057594037927936, 144115188075855872, 288230376151711744, 576460752303423488, 1152921504606846976, 2305843009213693952, 4611686018427387904, 9223372036854775808};
//     static constexpr const std::vector<unsigned long long> mask_right = {1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 1073741823, 2147483647, 4294967295, 8589934591, 17179869183, 34359738367, 68719476735, 137438953471, 274877906943, 549755813887, 1099511627775, 2199023255551, 4398046511103, 8796093022207, 17592186044415, 35184372088831, 70368744177663, 140737488355327, 281474976710655, 562949953421311, 1125899906842623, 2251799813685247, 4503599627370495, 9007199254740991, 18014398509481983, 36028797018963967, 72057594037927935, 144115188075855871, 288230376151711743, 576460752303423487, 1152921504606846975, 2305843009213693951, 4611686018427387903, 9223372036854775807, 18446744073709551615};
//     static constexpr const std::vector<unsigned long long> mask_left  = {18446744073709551614, 18446744073709551612, 18446744073709551608, 18446744073709551600, 18446744073709551584, 18446744073709551552, 18446744073709551488, 18446744073709551360, 18446744073709551104, 18446744073709550592, 18446744073709549568, 18446744073709547520, 18446744073709543424, 18446744073709535232, 18446744073709518848, 18446744073709486080, 18446744073709420544, 18446744073709289472, 18446744073709027328, 18446744073708503040, 18446744073707454464, 18446744073705357312, 18446744073701163008, 18446744073692774400, 18446744073675997184, 18446744073642442752, 18446744073575333888, 18446744073441116160, 18446744073172680704, 18446744072635809792, 18446744071562067968, 18446744069414584320, 18446744065119617024, 18446744056529682432, 18446744039349813248, 18446744004990074880, 18446743936270598144, 18446743798831644672, 18446743523953737728, 18446742974197923840, 18446741874686296064, 18446739675663040512, 18446735277616529408, 18446726481523507200, 18446708889337462784, 18446673704965373952, 18446603336221196288, 18446462598732840960, 18446181123756130304, 18445618173802708992, 18444492273895866368, 18442240474082181120, 18437736874454810624, 18428729675200069632, 18410715276690587648, 18374686479671623680, 18302628885633695744, 18158513697557839872, 17870283321406128128, 17293822569102704640, 16140901064495857664, 13835058055282163712, 9223372036854775808};
// };

/**
 * Classe feita para ser um interator em size_t.
 * É importante, pois no bint é necessário lidar com os valores dos índices, para realizar operações.
 * O valor de pos(), que é a posição do interator sempre está dentro do range permitido.
 * Caso os valores de ()++ ou ()-- levem a valores de pos() para fora do range, o valor da função
 * end() se torna true.
 */
class Iterator_Size_t {
    size_t _min_ = 0;
    size_t _size_; // se o valor é não declarado, o valor é std::numeric_limits<size_t>::max()
    size_t _pos_ = 0; // _min_ <= _pos < _size_
    bool _end_; // se o valor se encontra no limite.
    
  public:
  ///////////////////////////////////////////////////////////////////////////////////////
  // Constructor - implementation in .hpp file
  ///////////////////////////////////////////////////////////////////////////////////////
//   Iterator_Size_t();
  
  Iterator_Size_t(const size_t size = std::numeric_limits<size_t>::max(), 
            const size_t pos = 0,
            const size_t min = 0);
  
  ///////////////////////////////////////////////////////////////////////////////////////
  // implementation in .hpp file
  ///////////////////////////////////////////////////////////////////////////////////////
  inline bool end() const;
  inline size_t min() const;
  inline size_t size() const;
  inline size_t pos() const;
  inline size_t operator()() const; // esta função é igual ao pos() -> syntax sugar
  
  /**
   * Seta um novo limite máximo para o iterador.
   * @obs: o valor de pos() ou () NÃO é alterado.
   * O valor de min() NÃO é alterado.
   * Para que tal seja possível, é necessário o seguinte - a função verifica:
   * new_size > pos;
   * pos >= min.
   * Caso contrário, é lançado um erro.
   * @return o antigo valor de size que foi substituído pelo new_size.
   */
  inline size_t set_size(const size_t new_size);
  
  
  public:
  ///////////////////////////////////////////////////////////////////////////////////////
  // implementation in .cpp file
  ///////////////////////////////////////////////////////////////////////////////////////
  void set(const size_t size = std::numeric_limits<size_t>::max(), 
            const size_t pos = 0,
            const size_t min = 0);

  inline void set_reverse(const size_t size = std::numeric_limits<size_t>::max(),
            // const size_t pos = size -1, -> é setado automaticamente como pos = size -1
            const size_t min = 0);
  
  void set_pos(const size_t pos);

//   Iterator_Size_t operator+(const size_t num);
//   Iterator_Size_t operator-(const size_t num);
  size_t operator++();
  size_t operator--();
};

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .hpp file
///////////////////////////////////////////////////////////////////////////////////////
// Iterator_Size_t::Iterator_Size_t()
// {
//     set();
// }

Iterator_Size_t::Iterator_Size_t(
    const size_t size,
    const size_t pos,
    const size_t min)
{
    set(size, pos, min);
}

bool
Iterator_Size_t::end() const
{
    return _end_;
}

size_t
Iterator_Size_t::min() const
{
    return _min_;
}

size_t
Iterator_Size_t::size() const
{
    return _size_;
}

size_t
Iterator_Size_t::pos() const
{
    if(end()) {
        show("ERROR - iterator already out of scope.");
        exit(EXIT_FAILURE);
    }
    
    return _pos_;
}

size_t
Iterator_Size_t::operator()() const
{
    return pos();
}

void 
Iterator_Size_t::set_reverse(const size_t size, const size_t min)
{
    set(size, size -1, min);
}

size_t
Iterator_Size_t::set_size(const size_t new_size)
{
    if(new_size <= pos()) {
        show("ERROR - new_size must be greater than current position of iterator. new_size: ",new_size,", i(): ",pos(),"\n");
        exit(EXIT_FAILURE);
    }
    if(pos() < min()) {
        show("ERROR - minimum must be less or equal than current position of iterator. minimum: ",min(),", i(): ",pos(),"\n");
        exit(EXIT_FAILURE);
    }
    
    const auto old_size = size();
    _size_ = new_size;
    return old_size;
}
///////////////////////////////////////////////////////////////////////////////////////
// implementation in .cpp file
///////////////////////////////////////////////////////////////////////////////////////
void 
Iterator_Size_t::set(
    const size_t size,
    const size_t pos,
    const size_t min)
{
    if(size <= pos) {
        show("ERROR - size must be greater than pos. size: ",size,", pos: ",pos," - line: ",__LINE__,"\n");
        exit(EXIT_FAILURE);
    }
    if(pos < min) {
        show("ERROR - pos must be greater or equal than min. pos: ",pos,", init: ",min," - line: ",__LINE__,"\n");
        exit(EXIT_FAILURE);
    }
    
    _size_ = size;
    _pos_ = pos;
    _min_ = min;
    _end_ = false;
}

void 
Iterator_Size_t::set_pos(const size_t pos)
{
    if(_size_ <= pos) {
        show("ERROR - size must be greater than pos. size: ",_size_,", pos: ",pos," - line: ",__LINE__,"\n");
        exit(EXIT_FAILURE);
    }
    if(pos < _min_) {
        show("ERROR - pos must be greater or equal than min. pos: ",pos,", min: ",_min_," - line: ",__LINE__,"\n");
        exit(EXIT_FAILURE);
    }
    
    _pos_ = pos;
    _end_ = false;
}

size_t
Iterator_Size_t::operator++()
{
    if(end()) {
        show("ERROR - Interator already ended. Could not increment.");
        exit(EXIT_FAILURE);
    }
    
    if(_pos_+1 == _size_) {
        _end_ = true;
    } else {
        ++_pos_;
    }
    
    return _pos_;
}

size_t
Iterator_Size_t::operator--()
{
    if(end()) {
        show("ERROR - Interator already ended. Could not decrement.");
        exit(EXIT_FAILURE);
    }
    
    if(_pos_ == _min_) {
        _end_ = true;
    } else {
        --_pos_;
    }
    
    return _pos_;
}

///////////////////////////////////////////////////////////////////////////////////////
// bint internal types
/////////////////////////////////////////////////////////////////////////////////////// 
/**
 * Foi utilizado um alias, nos tipos fundamentais para que se possa se alterar a base da clase sem transtornos adicionais, já que várias operações demandam o tamanho do tipo (32, 64 bits, por exemplo).
 * A escolha da base da classe ser de 32 bits e não de 64bits, é porque várias operações são mais eficientes utilizando 32bits do que 64bits, devido ao overflow que gera, e são operações já feitas em hardware ao invés ter que implementar elas: ex: multiplicação, adição, subtração, potenciação, etc...
 */
using bint_base_t = uint_least32_t; // base = base do bint
using bint_uopr_t = uint_least64_t; // uopr = unsigned operation type
using bint_sopr_t = int_least64_t; // sopr = signed operation type

///////////////////////////////////////////////////////////////////////////////////////
// class bint
///////////////////////////////////////////////////////////////////////////////////////            
class bint
{
  const static int_least64_t _base_ = 4294967296; // 2^32
  const static int_least32_t _base_bit_size_ = 32; // for >> and <<
  const static int_least64_t _base_mask_ = 4294967295; // _base_ -1 -> último número que a base suporta -> & | ^, etc..
  const static int _base_str_size_ = 10;

  std::vector<bint_base_t> _number_;
  size_t _virtual_number_size_ = 0;
  bool _is_negative_ = false;
  bool _is_zero_ = true;
  
  public:
  ///////////////////////////////////////////////////////////////////////////////////////
  // implementation in .hpp file
  ///////////////////////////////////////////////////////////////////////////////////////
    bint() {};
    
    // inline bint(const int number);
    // inline bint(const long number);
    // inline bint(const long long number);
    // inline bint(const unsigned int number);
    // inline bint(const unsigned long number);
    // inline bint(const unsigned long long number);
    inline bint(const int_least32_t number);
    inline bint(const int_least64_t number);
    inline bint(const uint_least32_t number);
    inline bint(const uint_least64_t number);
    /**
     * @arg str_number_empty: se o @arg(number) for empty, então o valor do bint será o valor do @arg(str_number_empty).
     */
    inline bint(const std::string& number, const int_least64_t str_number_empty = 0);
    /**
     * @arg number_null: se o @arg(number) for NULL, então o valor do bint será o valor do @arg(number_null).
     */
    inline bint(const char* number, const int_least64_t number_null = 0);
    
  ///////////////////////////////////////////////////////////////////////////////////////
  // comparison operator functions ==, !=, <, >, <=, and >=
  ///////////////////////////////////////////////////////////////////////////////////////
    bool operator==(const bint& big_int) const;
    bool inline operator!=(const bint& big_int) const;
    bool operator<(const bint& big_int) const;
    bool inline operator>(const bint& big_int) const;
    bool inline operator<=(const bint& big_int) const;
    bool inline operator>=(const bint& big_int) const;

  ///////////////////////////////////////////////////////////////////////////////////////
  // logical operators &&, ||, !
  ///////////////////////////////////////////////////////////////////////////////////////
    bool operator&&(const bint& big_int) const;
    bool operator&&(const bool b) const;
    bool operator||(const bint& big_int) const;
    bool operator||(const bool b) const;
    bool operator!() const;
    
  ///////////////////////////////////////////////////////////////////////////////////////
  // operations operator functions +, -, *, /, %, div, pow, root
  ///////////////////////////////////////////////////////////////////////////////////////
    bint operator+(const bint& big_int) const;
    bint operator-(bint& big_int) const;
    bint operator*(const bint& big_int) const;
    bint inline operator/(const bint& divisor) const;
    bint inline operator%(const bint& divisor) const;

    inline bint& operator+=(const bint& big_int);
    inline bint& operator-=(bint& big_int);
    inline bint& operator*=(const bint& big_int);
    inline bint& operator/=(const bint& divisor);
    inline bint& operator%=(const bint& divisor);
    
    
    // Div<bint, long long> inline div(const int divisor) const;
    Div<bint, int_least64_t> ldiv(const int_least64_t divisor) const;
    Div<bint, bint> inline div(const bint& big_int) const {
        return div4(big_int);
    }
    
    // bint pow(const long long power) const;
    bint inline pow(const int_least64_t power) const;
    bint inline pow(const bint& power) const;
    
  ///////////////////////////////////////////////////////////////////////////////////////
  // operations operator binary functions >> << and or xor not
  ///////////////////////////////////////////////////////////////////////////////////////
  bint operator&(const bint& big_int) const;
  bint operator|(const bint& big_int) const;
  bint operator^(const bint& big_int) const;
  bint operator<<(const int_least64_t n) const;
  bint operator>>(const int_least64_t n) const;

  inline bint& operator&=(const bint& big_int);
  inline bint& operator|=(const bint& big_int);
  inline bint& operator^=(const bint& big_int);
  inline bint& operator<<=(const int_least64_t n);
  inline bint& operator>>=(const int_least64_t n);
  
  /**
   * Essa operação varia com o tamanho do número em relação a sua representação na base do bint.
   * Em uma representação na base 256.
   * Tem-se que a operação é feita por cada "casa" ou "bloco" de representação na base 256.
   * Isso varia o resultado, pois a casa toda é negada, e não somente á o último bit significativo do número, assim, o número 1: "00000001" -> ~1 -> "11111110" = 254 e não zero.
   * Atualmente essa negação é feita na base uint32_t.
   */
  bint operator~() const;
//   bint& inline operator~=() const;
  
  ///////////////////////////////////////////////////////////////////////////////////////
  // operations operator functions ++ --
  ///////////////////////////////////////////////////////////////////////////////////////
  inline bint& operator++();
  inline bint& operator--();
  
  ///////////////////////////////////////////////////////////////////////////////////////
  // operations operator functions others functions - implemented in different way
  ///////////////////////////////////////////////////////////////////////////////////////
  Div<bint, bint> div1(const bint& big_int) const;
  Div<bint, bint> div2(const bint& big_int) const;
  Div<bint, bint> div3(const bint& big_int) const;
  Div<bint, bint> div4(const bint& big_int) const;
  Div<bint, bint> div5(const bint& big_int) const;
  
  bint pow1(const int_least64_t power) const;
  bint pow1(const bint& power)  const;
  
  ///////////////////////////////////////////////////////////////////////////////////////
  // self operations
  // todas as self operations, retornam uma referência para a classe para as diferenciarem das 
  // operações normais e para que se possa fazer operações encadeadas.
  // ex: big_int.mul(546) + 36;
  // ex: big_int.mul(546).add(852);
  ///////////////////////////////////////////////////////////////////////////////////////
  bint& mul(const int_least32_t number);
  bint& add(const bint& number);
  bint& sub(const bint& number);
  bint& shift_l1(); // shift_left(1) -> de maneira otimizada
  bint& shift_left(const int_least64_t n);
  bint& shift_r1(); // shift_right(1) -> de maneira otimizada
  bint& shift_right(const int_least64_t n);
  inline bint& to_abs();

  ///////////////////////////////////////////////////////////////////////////////////////
  // others operations
  ///////////////////////////////////////////////////////////////////////////////////////
  bint gcd(const bint& big_int) const; // great common divisor

  ///////////////////////////////////////////////////////////////////////////////////////
  // auxilair functions
  ///////////////////////////////////////////////////////////////////////////////////////
    void set_zero();
    void print_in() const;
    void print_in_binary() const;
    /**
     * Verifica se o número é consistente.
     * _is_zero_, _virtual_number_size_ e _is_negative_.
     */
    bool check(const bool throw_exception = true);
    bool is_string_integer_number(const std::string& number, const bool throw_exception = false);
    
    /**
     * @return true -> if its is zero
     * false -> otherwise
     */
    bool inline zero() const;

    /**
     * @return true -> if the number is negative.
     * false -> otherwise.
     */
    bool inline negative() const;

    /**
     * 
     */
    // bint invert_sign() const;
    
    /**
     * Retorna a representação do número na base 10 em uma string.
     * Se o número não pode ser representado em uma string, então o comportamento é indefinido.
     */
    std::string str() const;
    
    /**
     * Retorna a representação binária do número em uma string.
     * a string retornada (str) contém o primeiro 1 como o bit mais significativo:
     * assim: str[0] = 1 -> e este 1 é o bit mais significativo.
     * Caso o número não possa ser representado em uma string o comportamento é indefinido.
     */
    std::string bstr() const;
    
    /**
     * Syntax sugar para retornar a representação em string do número na base 10.
     * Para ser utilizado com o operador std::cout << big_int.
     * simplemente faz: big_int.str();
     */
    inline friend std::ostream& operator<<(std::ostream& out, const bint& self) {
        out << self.str();
        return out;
    }
    
    // void inline set(const int number);
    // void inline set(const long number);
    // void inline set(const unsigned int number);
    // void inline set(const unsigned long number);
    void inline set(const int_least32_t number);
    void inline set(const uint_least32_t number);
    void set(const int_least64_t number);
    void set(const uint_least64_t number, const bool is_negative = false);
    /**
     * @arg str_number_empty: se o @arg(number) for empty, então o valor do bint será o valor do @arg(str_number_empty).
     */
    void set(const std::string& number, const int_least64_t str_number_empty = 0);
    void set2(const std::string& number); // old version - inneficient version
    
    void inline set2(const int_least64_t number); // set for long long optimized
    
    // bint& operator=(const bint& number);
    
    static Div<std::string, int> div_str_by_256(const std::string& numerator);
    Div<bint, int> div_by_10() const;
    
    // int cmp(const bint& n1, const bint& n2);
    /**
     * compara dois números do tipo bint e retorna a relação entre eles:
     * return:
     *   2 -> se o big_int this é o maior e tem sinais diferentes (number é negativo).
     *   1 -> se o big_int this é o maior e ambos tem o mesmo sinal.
     *   0 -> se o big_int this for igual ao number.
     *  -1 -> se o big_int this for menor que o number e tem os mesmo sinal.
     *  -2 -> se o big_int this for menor que o number e tem sinais diferentes (number é positivo).
     */
    int cmp(const bint& number) const;
    
    /**
     * compara os valores absolutos de dois números do tipo bint e retorna a relação entre eles:
     * return:
     *   1 -> se o big_int this é o maior.
     *   0 -> se o big_int this for igual ao number.
     *  -1 -> se o big_int this for menor que number.
     */
    int cmp_abs(const bint& number) const;
    
    /**
     * Valor absoluto do bint;
     */
    bint inline abs() const;

    /**
     * retorna um bool que representa o número big_int (this).
     * @return false -> if and only if zero() == true. -> if big_int(*this) = 0.
     * true -> otherwise.
     */
    bool inline to_bool() const;

    // template<typename TYPE>
    // bint min(const TYPE& big_int ...) const;

    // template<typename TYPE>
    // bint max(const TYPE& big_int ...) const;

    // template<typename TYPE>
    // bint& minr(const TYPE& big_int ...) const;

    // template<typename TYPE>
    // bint& maxr(const TYPE& big_int ...) const;
    
    /**
     * Transforma um número (char, int, long, long long) e suas versões unsigned
     * para a representação binária deles em string.
     * Exemplo: char = 1 = "00000001" -> sizeof(char)*8 = 8.
     */
    template<typename TYPE>
    std::string to_bstr(const TYPE number) const;
    
    /**
     * Encontra a posição do bit mais significativo em um número, dentro de um intervalo pasado.
     * Caso não exista um valor '1', é retornado -1.
     * O bit mais significativo é o primeiro bit de valor '1' do número, na representação big-endian.
     * @arg number: número em que será buscado.
     * @arg begin: índice do bit de busca.
     * @arg count: número de bits que serão buscados.
     *  caso o valor seja -1, todos os bits serão buscados: sizeof(TYPE)*8.
     * @arg sizeof_num_bit: quantos bits existe em um sizeof(1) = 1.
     * @return: o índice do bit mais significativo.
     * valores: >= 0 é o índice do bit mais significativo encontrado.
     * Esse índice deve estar na posição: (begin+@return)
     * -1: índice existe no número, mas não foi encontrado: exemplo: number = 0.
     * -2: índice inválido.
     */
     template<typename TYPE>
     int get_most_significat_bit(const TYPE number, const int count = -1, const int begin = 0, const int sizeof_num_bit = 8) const;
  ///////////////////////////////////////////////////////////////////////////////////////
  // funções para syntax sugar - para facilitar a interação com outros tipos.
  // implementation in .hpp file
  ///////////////////////////////////////////////////////////////////////////////////////
    template<typename TYPE>
    bint& operator=(const TYPE& number);
    
    template<typename TYPE>
    bool operator==(const TYPE& big_int) const;
    
    template<typename TYPE>
    bool operator!=(const TYPE& big_int) const;
    
    template<typename TYPE>
    bool operator<(const TYPE& big_int) const;
    
    template<typename TYPE>
    bool operator>(const TYPE& big_int) const;
    
    template<typename TYPE>
    bool operator<=(const TYPE& big_int) const;
    
    template<typename TYPE>
    bool operator>=(const TYPE& big_int) const;

    template<typename TYPE>
    bool operator&&(const TYPE& big_int) const;

    template<typename TYPE>
    bool operator||(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint operator+(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint operator-(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint operator*(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint operator/(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint operator%(const TYPE& big_int) const;

    template<typename TYPE>
    bint& operator+=(const TYPE& big_int);
    
    template<typename TYPE>
    bint& operator-=(const TYPE& big_int);
    
    template<typename TYPE>
    bint& operator*=(const TYPE& big_int);
    
    template<typename TYPE>
    bint& operator/=(const TYPE& big_int);
    
    template<typename TYPE>
    bint& operator%=(const TYPE& big_int);
    
    template<typename TYPE>
    Div<bint, bint> div(const TYPE& big_int) const;

    template<typename TYPE>
    Div<bint, bint> div1(const TYPE& big_int) const;

    template<typename TYPE>
    Div<bint, bint> div2(const TYPE& big_int) const;

    template<typename TYPE>
    Div<bint, bint> div3(const TYPE& big_int) const;

    template<typename TYPE>
    Div<bint, bint> div4(const TYPE& big_int) const;

    template<typename TYPE>
    Div<bint, bint> div5(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint pow(const TYPE& big_int) const;
    
    template<typename TYPE>
    bint& add(const TYPE& big_int);

    template<typename TYPE>
    bint& sub(const TYPE& big_int);

    template<typename TYPE>
    bint operator&(const bint& big_int) const;

    template<typename TYPE>
    bint operator|(const TYPE& big_int) const;

    template<typename TYPE>
    bint operator^(const TYPE& big_int) const;

    // template<typename TYPE>
    // bint operator<<(const int_least64_t n) const;

    // template<typename TYPE>
    // bint operator>>(const int_least64_t n) const;

    template<typename TYPE>
    bint& operator&=(const TYPE& big_int);

    template<typename TYPE>
    bint& operator|=(const TYPE& big_int);

    template<typename TYPE>
    bint& operator^=(const TYPE& big_int);

    // template<typename TYPE>
    // bint& operator<<=(const long long n);
    // template<typename TYPE>
    // bint& operator>>=(const long long n);

  private:
  ///////////////////////////////////////////////////////////////////////////////////////
  // auxiliar intern functions
  ///////////////////////////////////////////////////////////////////////////////////////
    /**
     * Atualiza o valor de _virtual_number_size_.
     * E caso ele fique fora do range de _number_, aloca mais memória para _number_.
     * @obs: NÃO ATUALIZA O VALOR DE _is_zero_. -> pois se pode colocar _number_.at(i) = 0 -> funções binárias e outras.
     */
    void inline increment_virtual_number_size();
    
    /**
     * insere o número na última posicao do array _number_ considerando o valor de _virtual_number_size_
     * Ao final atualiza o valor de _virtual_number_size_.
     * O número do tipo TYPE @arg(number) sofrerá um static cast para a última (considerando o valor de _virtual_number_size_) posição do vector.
     * Caso o valor de number não esteja no range de bint_base_t, o comportamento é indefinido.
     * O valor será colocado da seguinte forma (modo abreviado, pois é feito verificações adicionais):
     * check(.... _virtual_number_size_+1)
     * _number_.at(_virtual_number_size_) = static_cast<bint_base_t>(number);
     * Tal se dá pois esta função é privada e não publica.
     * @obs: NÃO ATUALIZA O VALOR DE _is_zero_. -> pois se pode colocar push_back(0) -> funções binárias e outras.
     */
    // void inline push_back(const int number);
    template<typename TYPE>
    void inline push_back(const TYPE number);
    
    /**
     * Atualiza o valor de _virtual_number_size_.
     * O valor é atualizado da seguinte maneira:
     * Procura o primeiro valor - em uma busca do final para o começo de _number_, tal que:
     * size_t i = size()-1;
     * e _virtual_number_size_ = i+1;
     * @arg begin: valor que começa a busca. Caso nenhum valor seja passado, a busca começará por _number_.size() -1.
     * Esse argumento é para otimização e os casos em que se tem certeza que o número resultante é menor que o original. ex: >> ou shift_right
     * @error: size >= _number_.size()
     * @obs: atualiza o valor de _is_zero_ e _is_negative_ também.
     */
     void inline update_virtual_number_size(const size_t begin = -1);
    
    // Div<std::string, int> div_str_by_256(const std::string& numerator);
    Div<int, int> inline div_number_part_by_divisor(const size_t pos, const int overflow, const int divisor);
    
    template<typename NUMBER_T>
    Div<int, int> div_number_part_by_divisor(
    NUMBER_T number, const int overflow,
    const int divisor, const int base) const;
    
    template<typename ARRAY>
    Div<ARRAY, int> div_number_by_part_manage(
    const ARRAY& number, const int base, 
    const int divisor, const size_t size) const;
    
    /**
     * Realiza a subtração de maneira otimizada.
     * Não faz conferências básicas como validade dos números.
     * Já assume que minuend >= subtrahend;
     */
    bint private_subtration(const bint& minuend, const bint& subtrahend) const;
    
    /**
     * Realiza a adição no próprio número de maneira otimizada.
     */
    void private_add(const bint& num);
    void private_sub_self_minuend(const bint& subtrahend);
    void private_sub_self_subtrahend(const bint& minuend);
    void private_sub_self_minuend_for_div(const bint& subtrahend, const size_t begin);

    /**
     * retorna a posição do bit mais significativo da posição de _virtual_number_size_.
     * se o número é zero() == true, então retorna 0.
     * As posições variam de 1 - _base_bit_size_ (para número não zero).
     */
    int get_most_significat_bit_pos() const;
    
    /**
     * Algoritmo para o bint::div3().
     * Compara o final do dividendo com o divisor, e verifica se é necessário
     * realizar um shift_l1() no divisor para realizar a divisão imediata ou se os
     * Ns primeiros bits do dividendo são iguais ao do divisor.
     * Ns primeiros bits são o número de bits do divisor.
     */
    int cmp_for_div(const bint& divisor, const int bit_shift_right) const;

    template<typename TYPE>
    bool check_template_arg_is_supported(const TYPE& str, const bool throw_exception = true) const;

    // std::tuple<bool, Div<bint, bint>> private_div_commom_cases(const bint& divisor);

    /**
     * Algoritmo para o bint::div4() e bint::div5()
     * Retorna os bits que correspondem ao número que será feito a subtração imediata de uma divisão do tipo "divisão longa".
     * O número é do tipo que sempre o resultado será 1.
     * Caso não seja possível mais dividir, o resultado da divisão será 0.
     * @result o número de bits necessários para encontrar um número no dividendo que seja imediatamente maior que o divisor.
     */
    size_t div_fill(const bint& divisor, const size_t divisor_size, const int divisor_most_signficant_bit_pos, const bint& dividend, size_t& dividend_size_pos, int& dividend_bit_pos);
    size_t div_fill(const bint& divisor, const int_least64_t divisor_num_bits, const bint& dividend, size_t& dividend_size_pos, int& dividend_bit_pos);
    size_t div_fill2(const bint& divisor, const int_least64_t divisor_num_bits, const bint& dividend, size_t& dividend_size_pos, int& dividend_bit_pos);
    
    /**
     * Para conformidade com os algorítimos implementados, a posição começa em 1.
     * @return o bit (0 ou 1) da posição em @arg(pos) do bloco (deve ser um tipo básico).
     */
    int get_bit(const uint_least32_t block, const int pos) const;

    /**
     * Para conformidade com os algorítimos implementados, a posição começa em 1.
     * @return retorna o número (unsigned int) que representa o bit da posição passada.
     * Ex: block: "0010" (2), pos: 2 -> return: 2;
     * Ex: block: "0010" (2), pos: 1 -> return: 0;
     * Ex: block: "0010" (2), pos: 3 -> return: 0;
     * Ex: block: "0110" (6), pos: 3 -> return: 4;
     * Ex: block: "1111" (15), pos: 3 -> return: 4;
     */
    inline uint_least32_t get_bit_int(const uint_least32_t block, const int pos) const;
};

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .hpp file
///////////////////////////////////////////////////////////////////////////////////////
// bint::bint(const int number) {
//     set2(static_cast<int_least64_t>(number));
// }

// bint::bint(const long number) {
//     set2(static_cast<int_least64_t>(number));
// }

// bint::bint(const long long number) {
//     set2(number);
// }

// bint::bint(const unsigned int number) {
//     set(static_cast<uint_least64_t>(number));
// }

// bint::bint(const unsigned long number) {
//     set(static_cast<uint_least64_t>(number));
// }

// bint::bint(const unsigned long long number) {
//     set(number);
// }

bint::bint(const int_least32_t number) {
    set(static_cast<int_least64_t>(number));
}

bint::bint(const int_least64_t number) {
    set(number);
}

bint::bint(const uint_least32_t number) {
    set(static_cast<uint_least64_t>(number));
}

bint::bint(const uint_least64_t number) {
    set(number);
}

// void bint::set(const int number) {
//     set2(static_cast<int_least64_t>(number));
// }

// void bint::set(const long number) {
//     set2(static_cast<int_least64_t>(number));
// }

void bint::set(const int_least32_t number) {
    set(static_cast<int_least64_t>(number));
}

void bint::set(const uint_least32_t number) {
    set(static_cast<uint_least64_t>(number));
}

void bint::set2(const int_least64_t number) {
    if(number == 0) {
        return set_zero();
    }
    
    int_least64_t n = number;
    if(number < 0) {
        n = -1*n;
        set(static_cast<uint_least64_t>(n), true);
        
    } else {
        set(static_cast<uint_least64_t>(n));
    }
}

// void bint::set(const unsigned int number) {
//     set(static_cast<uint_least64_t>(number));
// }

// void bint::set(const unsigned long number) {
//     set(static_cast<uint_least64_t>(number));
// }

bint::bint(const std::string& number, const int_least64_t str_number_empty) {
    set(number, str_number_empty);
}

bint::bint(const char* number, const int_least64_t number_null) {
    if(number == nullptr ) {
        set(number_null);
    } else {
        set(static_cast<std::string>(number));
    }
}

void bint::increment_virtual_number_size() {
    if(_virtual_number_size_ == std::numeric_limits<size_t>::max()) {
        throw binterror("maximun value for number size. Maximum value for _virtual_number_size_: ",std::numeric_limits<size_t>::max());
        // show("ERROR - Maximun value for number size. Maximum value for _virtual_number_size_: ",std::numeric_limits<size_t>::max(),", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    ++_virtual_number_size_; // increment
    
    while(_virtual_number_size_ >= _number_.size()) {
        const size_t new_size = 256+_number_.size(); // atention to check this point
        _number_.resize(new_size);
    }
}

bool bint::operator!=(const bint& big_int) const {
    return !(this->operator==(big_int));
}

bool bint::operator>(const bint& big_int) const {
    if(this->operator==(big_int)) return false;
    return !(this->operator<(big_int));
}

bool bint::operator<=(const bint& big_int) const {
    if(this->operator==(big_int)) return true;
    return this->operator<(big_int);
}

bool bint::operator>=(const bint& big_int) const {
    if(this->operator==(big_int)) return true;
    return !(this->operator<(big_int));
}

bool bint::operator&&(const bint& big_int) const {
    if(zero()) {
        return false;
    }
    if(big_int.zero()) {
        return false;
    }
    return true;
}

bool bint::operator&&(const bool b) const {
    if(zero()) {
        return false;
    }
    return b;
}

bool bint::operator||(const bint& big_int) const {
    if(zero() == false) {
        return true;
    }
    if(big_int.zero() == false) {
        return true;
    }
    return false;
}

bool bint::operator||(const bool b) const {
    if(zero() == false) {
        return true;
    }
    return b;
}

bool bint::operator!() const {
    if(zero()) {
        return true;
    }
    return false;
}

bint bint::operator/(const bint& divisor) const {
    const auto result = div(divisor);
    return result.quot;
}

bint bint::operator%(const bint& divisor) const {
    const auto result = div(divisor);
    return result.rem;
}

bint& bint::operator+=(const bint& big_int) {
    *this = this->operator+(big_int);
    return *this;
}

bint& bint::operator-=(bint& big_int) {
    *this = this->operator-(big_int);
    return *this;
}

bint& bint::operator*=(const bint& big_int) {
    *this = this->operator*=(big_int);
    return *this;
}

bint& bint::operator/=(const bint& divisor) {
    *this = this->operator/(divisor);
    return *this;
}

bint& bint::operator%=(const bint& divisor) {
    *this = this->operator%(divisor);
    return *this;
}

bint& bint::operator&=(const bint& big_int) {
    *this = this->operator&(big_int);
    return *this;
}

bint& bint::operator|=(const bint& big_int) {
    *this = this->operator|(big_int);
    return *this;
}

bint& bint::operator^=(const bint& big_int) {
    *this = this->operator^(big_int);
    return *this;
}

bint& bint::operator<<=(const int_least64_t n) {
    *this = this->operator<<(n);
    return *this;
}

bint& bint::operator>>=(const int_least64_t n) {
    *this = this->operator>>(n);
    return *this;
}

bint bint::pow(const int_least64_t power) const {
    return pow1(power);
}

bint bint::pow(const bint& power) const {
    return pow1(power);
}

bint& bint::operator++() {
    return add(1);
}

bint& bint::operator--() {
    return add(-1);
}

bint& bint::to_abs() {
    if(_is_negative_ ==  true) {
        _is_negative_ = false;
    }
    return *this;
}

void bint::update_virtual_number_size(const size_t begin) {
    if(_number_.empty()) {
        set_zero();
        return;
    }
    
    size_t i = begin == -1 ? _number_.size() -1 : begin;
    for(; i != -1; --i) {
        if(_number_.at(i) != 0) {
            _virtual_number_size_ = i+1;
            _is_zero_ = false;
            return;
        }
    }

    set_zero(); // o número é zero

    // auto i = Iterator_Size_t(_number_.size(), _number_.size()-1);
    // for(; i.end() == false; --i) {
    //     if(_number_.at(i()) != 0) {
    //         _virtual_number_size_ = i()+1;
    //         _is_zero_ = false;
    //         return;
    //     }
    // }
    // set_zero(); // o número é zero
}

bool bint::zero() const {
    return _is_zero_;
    // if(_is_zero_ == true) {
    //     if(_is_negative_ == false) {
    //         return true;
    //     } else {
    //         show("ERROR - zero cannot be a negative number. -- LINE: ",__LINE__,"\n");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    
    // return false;
}

bool bint::negative() const {
    return _is_negative_;
}

bint bint::abs() const {
    // bint aux ("0");
    // if(*this >= aux) {
    //     return *this;
    // } else {
    //     aux = *this;
    //     aux._is_negative_ = false;
    //     return aux;
    // }
    if(negative() == true) {
        auto a = *this;
        a._is_negative_ = false;
        return a;
    } else {
        return *this;
    }
}

bool bint::to_bool() const {
    return zero() == true ? false : true; // return !zero();
}


///////////////////////////////////////////////////////////////////////////////////////
// implementation in .hpp file
// template functions - private functions
///////////////////////////////////////////////////////////////////////////////////////
template<typename TYPE>
void bint::push_back(const TYPE number) {
    const auto last_id = _virtual_number_size_;
    increment_virtual_number_size();
    // typename std::decay<decltype(_number_.at(0))>::type -> devolve o tipo que é o array, no caso: unsigned char
    _number_.at(last_id) = static_cast<bint_base_t>(number);
}

template<typename TYPE>
bool bint::check_template_arg_is_supported(const TYPE& str, const bool throw_exception) const {
    if constexpr(std::is_pointer<TYPE>::value)  {
		if(str == nullptr || str == NULL) {
			if(throw_exception == true) {
                throw binterror("pointer representing string integer number cannot be NULL.");
	            // show("ERROR - pointer representing string integer number cannot be NULL. - line: ",__LINE__,"\n");
	            // exit(EXIT_FAILURE);
	        } else {
	            return false;
	        }   
		}
	}
	return true;
    
    /**
     * TODO - achar uma forma de fazer as strings literais (char[2], char[3], ..., char[N]) passar no teste std::is_same<TYPE, char[N]>::value, sem colocar o valor exato delas.
     * Enquanto isso, essa parte do código foi comentado.
     */
     /*
    if constexpr(std::is_same<TYPE, std::string>::value) {
		return true;
	} else if constexpr(std::is_same<TYPE, std::string_view>::value) {
	    return true;
	} else if constexpr(std::is_same<TYPE, char*>::value) {
	    return true;
	} else if constexpr(std::is_same<TYPE, char[2]>::value) {
	    return true;
	} else {
	    if(throw_exception == true) {
	        int status;
            std::string tname = typeid(TYPE).name();
	        char *demangled_name = abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
            if(status == 0) {
                tname = demangled_name;
                std::free(demangled_name);
            }
            show("ERROR - TYPE is not a std::string or std::string_view or char*. TYPE: '",tname,"', line: ",__LINE__,"\n");
	       // show("ERROR - TYPE is not a std::string or std::string_view. TYPE: '",typeid(TYPE).name(),"', line: ",__LINE__,"\n");
	        exit(EXIT_FAILURE);
	    } else {
	        return false;
	    }
	}
*/
}

template<typename TYPE>
std::string bint::to_bstr(const TYPE number) const {
    const int total_bits = sizeof(number) * 8; // considera que 1 byte tem 8 bits.
    
    std::string str = "";
    for(int i=0; i < total_bits; ++i) {
        const auto b = (number >> i) & 1;
        str = std::to_string(b) + str;
    }
    
    return str;
}

template<typename TYPE>
int bint::get_most_significat_bit(
    const TYPE number, const int count,
    const int begin, const int sizeof_num_bit) const {
    if(number == 0) {
        return -1;
    }
    if(begin < 0) {
        throw binterror("begin must be equal or greater than 0. begin: ",begin);
        // show("ERROR - begin must be equal or greater than 0. begin: ",begin,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(count < 1 && count != -1) {
        throw binterror("count must be equal or greater than 1. count: ",count);
        // show("ERROR - count must be equal or greater than 1. count: ",count,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(sizeof_num_bit < 1) {
        throw binterror("sizeof(1) numbers of bits  must be equal or greater than 1. sizeof_num_bit: ",sizeof_num_bit);
        // show("ERROR - sizeof(1) numbers of bits  must be equal or greater than 1. sizeof_num_bit: ",sizeof_num_bit,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    
    const int total_bits = sizeof(number) * sizeof_num_bit;
    if(begin >= total_bits) {
        throw binterror("begin must be less than number of totals bits of number. begin: ",begin);
        // show("ERROR - begin must be less than number of totals bits of number. begin: ",begin,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if((begin+count) >= total_bits && count != -1) {
        throw binterror("the range of the search must be equal or less than total of bits. begin: ",begin,", count: ",count,", total number of bits: ",total_bits);
        // show("ERROR - the range of the search must be equal or less than total of bits. begin: ",begin,", count: ",count,", total number of bits: ",total_bits,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    
    const int last_bit_id = count == -1 ? total_bits -1 : count;
    
    for(int i=last_bit_id; i > -1; --i) {
        const auto n = number >> i;
        const auto bit = n & 1;
        
        if(bit == 1) {
            return i;
        }
    }
    
    return -1;
}
///////////////////////////////////////////////////////////////////////////////////////
// implementation in .hpp file
// string syntax sugar
///////////////////////////////////////////////////////////////////////////////////////
template<typename TYPE>
bint& bint::operator=(const TYPE& number) {
    check_template_arg_is_supported(number);
    set(number);
    return *this;
}

template<typename TYPE>
bool bint::operator==(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator==(n);
}
    
template<typename TYPE>
bool bint::operator!=(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator!=(n);
}
    
template<typename TYPE>
bool bint::operator<(const TYPE& big_int) const{
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator<(n);
}
    
template<typename TYPE>
bool bint::operator>(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator>(n);
}
    
template<typename TYPE>
bool bint::operator<=(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator<=(n);
}
    
template<typename TYPE>
bool bint::operator>=(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator>=(n);
}

template<typename TYPE>
bool bint::operator&&(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);

    if constexpr(std::is_same<TYPE, unsigned char>::value) {
		const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    } else if constexpr(std::is_same<TYPE, char>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    } else if constexpr(std::is_same<TYPE, unsigned int>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    } else if constexpr(std::is_same<TYPE, int>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    } else if constexpr(std::is_same<TYPE, unsigned long>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    } else if constexpr(std::is_same<TYPE, long>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    } else if constexpr(std::is_same<TYPE, unsigned long long>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator&&(v);
    }

    bint n (big_int);
    return operator&&(n);
}

template<typename TYPE>
bool bint::operator||(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);

    if constexpr(std::is_same<TYPE, unsigned char>::value) {
		const bool v = big_int == 0 ? false : true;
        return operator||(v);
    } else if constexpr(std::is_same<TYPE, char>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator||(v);
    } else if constexpr(std::is_same<TYPE, unsigned int>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator||(v);
    } else if constexpr(std::is_same<TYPE, int>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator||(v);
    } else if constexpr(std::is_same<TYPE, unsigned long>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator||(v);
    } else if constexpr(std::is_same<TYPE, long>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator||(v);
    } else if constexpr(std::is_same<TYPE, unsigned long long>::value) {
        const bool v = big_int == 0 ? false : true;
        return operator||(v);
    }

    bint n (big_int);
    return operator||(n);
}

template<typename TYPE>
bint bint::operator+(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator+(n);
}
    
template<typename TYPE>
bint bint::operator-(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator-(n);
}
    
template<typename TYPE>
bint bint::operator*(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator*(n);
}
    
template<typename TYPE>
bint bint::operator/(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator/(n);
}
    
template<typename TYPE>
bint bint::operator%(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator%(n);
}

template<typename TYPE>
bint& bint::operator+=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator+=(n);
}
    
template<typename TYPE>
bint& bint::operator-=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator-=(n);
}
    
template<typename TYPE>
bint& bint::operator*=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator*=(n);
}
    
template<typename TYPE>
bint& bint::operator/=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator/=(n);
}
    
template<typename TYPE>
bint& bint::operator%=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator%=(n);
}

template<typename TYPE>
Div<bint, bint> bint::div(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return div(n);
}

template<typename TYPE>
Div<bint, bint> bint::div1(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return div1(n);
}

template<typename TYPE>
Div<bint, bint> bint::div2(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return div2(n);
}

template<typename TYPE>
Div<bint, bint> bint::div3(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return div3(n);
}

template<typename TYPE>
Div<bint, bint> bint::div4(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return div4(n);
}

template<typename TYPE>
Div<bint, bint> bint::div5(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return div5(n);
}

template<typename TYPE>
bint bint::pow(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    
    // if constexpr(std::is_same<TYPE, char>::value) {
	// 	return pow(static_cast<int_least64_t>(big_int));
	// } else if constexpr(std::is_same<TYPE, unsigned char>::value) {
	//     return pow(static_cast<int_least64_t>(big_int));
	// } else if constexpr(std::is_same<TYPE, int>::value) {
	//     return pow(static_cast<int_least64_t>(big_int));
	// } else if constexpr(std::is_same<TYPE, unsigned int>::value) {
	//     return pow(static_cast<int_least64_t>(big_int));
	// } else if constexpr(std::is_same<TYPE, long>::value) {
	//     return pow(static_cast<int_least64_t>(big_int));
	// } else if constexpr(std::is_same<TYPE, long long>::value) {
	//     return pow(static_cast<int_least64_t>(big_int));
    // }
    if constexpr(std::is_same<TYPE, int_least32_t>::value) {
		return pow(static_cast<int_least64_t>(big_int));
	} else if constexpr(std::is_same<TYPE, uint_least32_t>::value) {
	    return pow(static_cast<int_least64_t>(big_int));
    }

    bint n (big_int);
    return pow(n);
}

template<typename TYPE>
bint& bint::add(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return add(n);
}

template<typename TYPE>
bint& bint::sub(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return sub(n);
}

template<typename TYPE>
bint bint::operator&(const bint& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator&(n);
}

template<typename TYPE>
bint bint::operator|(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator|(n);
}

template<typename TYPE>
bint bint::operator^(const TYPE& big_int) const {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator^(n);
}

template<typename TYPE>
bint& bint::operator&=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator&=(n);
}

template<typename TYPE>
bint& bint::operator|=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator|=(n);
}

template<typename TYPE>
bint& bint::operator^=(const TYPE& big_int) {
    check_template_arg_is_supported(big_int);
    bint n (big_int);
    return operator^=(n);
}

///////////////////////////////////////////////////////////////////////////////////////
// Error class - header functions
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// Error class - inline
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// Error class - templates
///////////////////////////////////////////////////////////////////////////////////////

#endif //__BORGES_BINT_HPP__

