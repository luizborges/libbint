#include "bint.hpp"

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .cpp file
///////////////////////////////////////////////////////////////////////////////////////
void bint::set(const long long number) {
    set_zero(); // seta o número para zero
    
    if(number == 0) {
        return;
    }
    _is_zero_ = false;
    
    if(number < 0) {
        _is_negative_ = true;
    }
    
    if(_number_.empty()) {
        _number_.resize(256);
    }
    
    auto quotient = number < 0 ? -1*number : number;
    auto division = std::lldiv(quotient, this->_base_);
    push_back(division.rem);
    
    if(division.quot != 0) {
    
        while(division.quot != 0) {
            division = std::lldiv(division.quot, this->_base_);
            push_back(division.rem);
        }
    }
}

void bint::set(const unsigned long long number, const bool is_negative) {
    set_zero(); // seta o número para zero
    
    if(number == 0) {
        return;
    }
    _is_zero_ = false;
    _is_negative_ = is_negative; // para utilizar em transformações int, long, long long, ...
    
    // if(number < 0) { // unsigned -> nunca o número será negativo
    //     _is_negative_ = true;
    // }
    
    if(_number_.empty()) {
        _number_.resize(256);
    }
    
    const int size = sizeof(unsigned long long);
    
    unsigned long long num = number;
    for(int i=0; i < size; ++i) {
        const int r = static_cast<int>(num & _base_mask_);
        push_back(r);
        
        num = num >> _base_bit_size_; // considera 1 byte = 8 bits
    }
    
    // atualiza o valor de _virtual_number_size_
    // show("B::vns: ",_virtual_number_size_,"\n");
    for(int i = size-1; i > -1; --i) {
        if(_number_.at(i) != 0) {
            _virtual_number_size_ = i+1;
            break;
        }
    }
    // show("A::vns: ",_virtual_number_size_,"\n");
}

/**
 * @obs: strings vazias NÃO são tratadas como sendo o número zero. - emite erro
 * @obs: são considerados números na base 10.
 */
void bint::set2(const std::string& number) {
    set_zero(); // reset number
    
    if(number.empty() == true) {
        show("ERROR: LINE: ",__LINE__,"\n");
        exit(EXIT_FAILURE);
    }
    
    // verifica se o número de fato é uma string que representa um número inteiro
    if(is_string_integer_number(number) == false) {
        show("ERROR - string is not a number. - string: '",number,"' - line: ", __LINE__,"\n");
        exit(EXIT_FAILURE);
    } 
    
    if(number == "0") {
        return;
    } else {
        _is_zero_ = false;
    }
    
    if(_number_.empty()) {
        _number_.resize(256);
    }
    
    /////////////////////////////////////////////////////////////////
    // verifica se o número é negativo
    /////////////////////////////////////////////////////////////////
    size_t init_copy = 0;
    if(number.front() == '-') {
        init_copy = 1;
        _is_negative_ = true;  // seta o número como negativo
    }
    
    /////////////////////////////////////////////////////////////////
    // realiza a divisão propriamente
    /////////////////////////////////////////////////////////////////
    // _virtual_number_size_ = 0; // é resetado devido a função set_zero()
    std::string num = number.substr(init_copy);
    do {
        auto result = div_str_by_256(num);
        num = result.quot;
        _number_.at(_virtual_number_size_) = static_cast<unsigned char>(result.rem);
        increment_virtual_number_size();
    } while(num != "0");
    
}

void bint::set(const std::string& number, const long long str_number_empty) {
    set_zero(); // reset number
    
    if(number.empty() == true) {
        set(str_number_empty);
    }
    
    if(number == "0") {
        return;
    }
    
    if(is_string_integer_number(number) == false) {
        throw binterror("NOT a number. number: '",number,"'");
        // show("ERROR. NOT a number. number: '",number,"', line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    
    /////////////////////////////////////////////////////////////////
    // verifica se o número é negativo
    /////////////////////////////////////////////////////////////////
    std::string_view str_int {number};
    const bool is_negative = number.at(0) == '-' ? true : false;
    if(is_negative) {
        str_int = str_int.substr(1); // retira o character '-'
        _is_negative_ = true; // seta o número como negativo
    }
    
    /////////////////////////////////////////////////////////////////
    // realiza a mudança de base propriamente
    /////////////////////////////////////////////////////////////////
    const int fd = static_cast<int>(str_int.back()) - '0';
    push_back(fd);
    bint ten_power = 1;
    auto it = str_int.rbegin();
    ++it; // tira o último elemento que já foi contabilizado.
    for(; it != str_int.rend(); ++it) {
        ten_power.mul(10);
        
        if(*it == '0') {
            continue;
        }
        
        const int digit = static_cast<int>(*it) - '0';
        int overflow_10 = 0;
        int overflow    = 0;
        size_t i = 0;
        for(; i < ten_power._virtual_number_size_; ++i) {
            if(i >= _number_.size()) {
                _number_.resize(i+256, 0);
            }
            
            if(_number_.at(i) == 0 && ten_power._number_.at(i) == 0 
                && overflow == 0 && overflow_10 == 0) {
                continue;
            }
            
            const int a = digit * ten_power._number_.at(i) + overflow_10;
            const int b = a & _base_mask_;
            overflow_10 = a >> _base_bit_size_;
            
            const int r1 = b + _number_.at(i) + overflow;
            const int r2 = r1 & _base_mask_;
            overflow = r1 >> _base_bit_size_;
            _number_.at(i) = r2;
        }
        
        while(overflow > 0 || overflow_10 > 0) {
            if(i >= _number_.size()) {
                _number_.resize(i+256, 0);
            }
            
            const int r = _number_.at(i) + overflow_10 + overflow;
            const int r1 = r & _base_mask_;
            overflow = r1 >> _base_bit_size_;
            overflow_10 = overflow_10 >> _base_bit_size_;
            _number_.at(i) = r1;
            ++i; // atualiza o i
        }
    }
    
    
    update_virtual_number_size();
    if(zero()) {
        throw binterror("number here must NOT be zero. number: ",number);
        // show("ERROR - number cannot be zero. number: ",number,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
}

Div<std::string, int> 
bint::div_str_by_256(const std::string& numerator) {
    // /////////////////////////////////////////////////////////////////
    // // verifica entradas
    // /////////////////////////////////////////////////////////////////
    // Str_Int numerator_si (numerator);
    // // Str_Int::is_string_integer_number(numerator);
    // // if(divisor == 0) {
    // //     show("ERROR - division by 0.\n");
    // //     abort();
    // // }
    // // if(divisor_str_size < 1) {
    // //     show("ERROR - division string size is lesser than 1.\n");
    // //     abort();
    // // }
    
    // /////////////////////////////////////////////////////////////////
    // // coloca todos os números como sendo positivos
    // /////////////////////////////////////////////////////////////////
    // bool is_negative_numerator = numerator_si.is_negative() ? true : false;
    // if(is_negative_numerator) numerator_si = numerator_si.abs();
    
    // /////////////////////////////////////////////////////////////////
    // // realiza a divisão
    // /////////////////////////////////////////////////////////////////
    // const auto max_llong_t = std::numeric_limits<long long>::max();
    // const auto max_llong_str = std::to_string(max_llong_t);
    // const Str_Int max_llong (max_llong_str);
    // const Str_Int base256 ("256");
    // // size_t pos = 0;
    // std::string num = numerator_si.str();
    // std::string rem = "";
    // std::string quot = "";
    
    // /////////////////////////////////////////////////////////////////
    // // realiza a primeira iteração da divisão
    // /////////////////////////////////////////////////////////////////
    // const auto dividend1 = Str_Int(num).div_cut_numerator_by_celing(0, max_llong);
    // if(dividend1.empty()) {
    //     show("ERROR - dividend cannot be an empty string number. - cut by number: ",num,"'\n");
    //     abort();
    // }
    // const auto dividend_ll1 = std::stoll(dividend1);
    // const auto res1 = std::lldiv(dividend_ll1, 256);
    // quot = std::to_string(res1.quot); // atualiza o quociente da divisão
    // rem = std::to_string(res1.rem); // atualiza o novo resto da divisão
    
    // num = num.substr(dividend1.size()); // atualiza o número que ainda resta para a divisão
    
    // // show("first iteraction::\n");
    // // show("num: \t",num," | size: ",num.size(),"\n");
    // // show("rem: \t",rem," | size: ",rem.size(),"\n");
    // // show("quot: \t",quot," | size: ",quot.size(),"\n\n");
    
    
    // /////////////////////////////////////////////////////////////////
    // // realiza as outras iterações da divisão
    // /////////////////////////////////////////////////////////////////
    // while(num.empty() == false) 
    // {
    //     const auto dividend = Str_Int::div_cut_numerator_by_celing(
    //         num, 0, max_llong, Str_Int(rem), base256);
    //     // show("num: \t",num," | size: ",num.size(),"\n");
    //     // show("rem: \t",rem," | size: ",rem.size(),"\n");
    //     // show("new_num: \t",new_num," | size: ",new_num.size(),"\n");
    //     // show("dividend: \t",dividend," | size: ",dividend.size(),"\n");
    //     // show("max_llong_str: \t",max_llong_str," | size: ",max_llong_str.size(),"\n");
    //     if(dividend.empty()) {
    //         show("ERROR - dividend cannot be an empty string number. - cut by number: ",num," | rem: '",rem,"'\n");
    //         abort();
    //     }
    //     const size_t cut_num = rem == "0" ? dividend.size() : dividend.size() - rem.size();
    //     const auto dividend_ll = std::stoll(dividend);
    //     const auto res = std::lldiv(dividend_ll, 256);
    //     quot = quot + std::to_string(res.quot); // atualiza o quociente da divisão
    //     rem = std::to_string(res.rem); // atualiza o novo resto da divisão
        
    //     // show("res.quot: \t",res.quot,"\n");
    //     // show("quot_str: \t",std::to_string(res.quot),"\n");
    //     // show("res.rem: \t",res.rem,"\n");
    //     // show("rem: \t",rem,"\n\n");
        
    //     if(cut_num < num.size()) {
    //         num = num.substr(cut_num); // atualiza o número que ainda resta para a divisão
    //         continue; // otimização
    //     } else if(cut_num == num.size()) {
    //         break; // a string ficará vazia, por questão de otimização - apenas insere o break
    //     } else {
    //         show("ERROR - cut_num: ",cut_num," | num.size(): ",num.size());
    //         // abort();
    //         exit(1);
    //     }
    // };
    
    // /////////////////////////////////////////////////////////////////
    // // seta os resultados
    // /////////////////////////////////////////////////////////////////
    // int rem_i = std::stoi(rem); // sempre menor que 256
    // if(is_negative_numerator) {
    //     rem_i = rem_i * -1;
    //     quot = quot == "0" ? "0" : "-" + quot;
    // }
    
    // return { quot, rem_i };
    return {numerator, 0};
}

void bint::set_zero() {
    for(auto& e : _number_) {
        e = 0;
    }
    _virtual_number_size_ = 0;
    _is_negative_ = false;
    _is_zero_ = true;
}

bool bint::is_string_integer_number(const std::string& str, const bool throw_exception) {
    if(str.empty() == true) {
        if(throw_exception == true) {
            throw binterror("empty string number");
            // show("ERROR: empty string number - LINE: ",__LINE__,"\n");
            // exit(EXIT_FAILURE);
        } else {
            return false;
        }
    }
    if(str == "0") return true;
    
    size_t i = 0; // posição inicial da string que será checada
    
    /////////////////////////////////////////////////////////////////
    // verifica se o número é negativo
    /////////////////////////////////////////////////////////////////
    if(str.at(0) == '-') {
        if(str.size() < 2) {
            if(throw_exception == true) {
                throw binterror("malformed number. number: '",str,"'");
                // show("ERROR: malformed number. number: '",str,"' - LINE: ",__LINE__,"\n");
                // exit(EXIT_FAILURE);
            } else {
                return false;
            }
        }
        i = 1;
    }
    
    /////////////////////////////////////////////////////////////////
    // verifica o primeiro dígito do número é válido - não pode ser 0
    /////////////////////////////////////////////////////////////////
    const std::string number_init = "123456789";
    bool check = false;
    for(size_t j=0; j < number_init.size(); ++j) {
        if(str.at(i) == number_init.at(j)) {
            check = true;
            ++i;
            break;
        }
    }
    
    if(check == false) {
        if(throw_exception == true) {
            throw binterror("malformed number. number: '",str,"'");
            // show("ERROR: LINE: ",__LINE__,"\n");
            // exit(EXIT_FAILURE);
        } else {
            return false;
        }
    }
    
    /////////////////////////////////////////////////////////////////
    // verifica os demais dígitos do número - pode ser zero
    /////////////////////////////////////////////////////////////////
    for(; i < str.size(); ++i) {
        check = false;
        const std::string number = "0123456789";
        for(size_t j=0; j < number.size(); ++j) {
            if(str.at(i) == number.at(j)) {
                check = true;
                break;
            }
        }
        
        if(check == false) {
            if(throw_exception == true) {
                throw binterror("malformed number. number: '",str,"'");
                // show("ERROR: LINE: ",__LINE__,"\n");
                // exit(EXIT_FAILURE);
            } else {
                return false;
            }
        }
    }
    
    return true;
}

void bint::print_in() const {
    // show("this: ",this,"\n");
    show("base: ",_base_,", base str size: ",_base_str_size_,"\n");
    show("base bits size: ",_base_bit_size_,", base max number: ",_base_mask_,"\n");
    show("is negative: ",_is_negative_,", is zero: ",_is_zero_,"\n");
    show("virtual_number_size: ",_virtual_number_size_,", real number size ",_number_.size(),"\n");
    
    if(_number_.empty()) {
        show("[]");
    } else {
        // for(size_t i = _virtual_number_size_ -1; i > 0; --i) {
        for(size_t i = _number_.size() -1; i > 0; --i) {
            const int z = static_cast<int>(_number_.at(i));
            // show("[",i,":",z,"]");
            show("[",z,"]");
        }
        const int z = static_cast<int>(_number_.at(0));
        // show("[",0,":",z,"]");
        show("[",z,"]");
    }
    show("\n");
}

void bint::print_in_binary() const {
    show("base: ",_base_,", base str size: ",_base_str_size_,"\n");
    show("is negative: ",_is_negative_,", is zero: ",_is_zero_,"\n");
    show("virtual_number_size: ",_virtual_number_size_,", real number size ",_number_.size(),"\n");
    
    if(_number_.empty()) {
        show("[]");
    } else {
        for(size_t i = _virtual_number_size_ -1; i > 0; --i) {
        // for(size_t i = _number_.size() -1; i > 0; --i) {
            const auto z = to_bstr(_number_.at(i));
            // const int z = static_cast<int>(_number_.at(i));
            // // show("[",i,":",z,"]");
            show("[",z,"]");
        }
        const auto z = to_bstr(_number_.at(0));
        // show("[",0,":",z,"]");
        show("[",z,"]");
    }
    show("\n");
}

std::string bint::str() const {
    if(_is_zero_) return "0";
    
    std::vector<char> number_inverse;
    for(bint dividend = *this; true; ) {
        const auto result = dividend.div_by_10();
        number_inverse.push_back(static_cast<char>(std::abs(result.rem)));
        
        if(result.quot._is_zero_ == true) break; // verifica o fim do loop
        
        dividend = result.quot; // atualiza o dividend
    }
    
    std::string str = "";
    // Iterator_Size_t i;
    // for(i.set_reverse(number_inverse.size()); i.end() == false; --i) {
        // const int n = static_cast<int>(number_inverse.at(i()));
        // str = str + std::to_string(n);
    // }
    for(size_t i = number_inverse.size() -1; i != -1; --i) {
        const int n = static_cast<int>(number_inverse.at(i));
        str = str + std::to_string(n);
    }
    
    if(_is_negative_) return "-" + str;
    return str;
}

std::string bint::bstr() const {
    if(zero()) {
        return "0";
    }
    
    if(_virtual_number_size_ == 0) {
        throw binterror("_virtual_number_size_ can NOT be zero here.");
        // show("ERROR - line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    
    /////////////////////////////////////////
    // get the first chunck of number
    /////////////////////////////////////////
    auto i = _virtual_number_size_ -1;
    int msb = get_most_significat_bit(_number_.at(i));
    if(msb < 0) {
        throw binterror("most significant bit must exists for numbers different from 0.");
        // show("ERROR - most significant bit must exists for numbers different from 0. line: ",__LINE__,"\n");
    }
    
    std::string bnum = "1";
    for(int j=msb-1; j > -1; --j) {
        const auto n = _number_.at(i) >> j;
        const auto bit = n & 1;
        const char c = bit == 1 ? '1' : '0';
        bnum = bnum + c;
    }
    
    if(i == 0) { // check the end
        return bnum;
    }
    
    // auto it = Iterator_Size_t();
    // it.set_reverse(i);
    // for(; it.end() == false; --it) {
    //     for(int j=_base_bit_size_-1; j > -1; --j) {
    //         const auto n = _number_.at(it()) >> j;
    //         const auto bit = n & 1;
    //         const char c = bit == 1 ? '1' : '0';
    //         bnum = bnum + c;
    //     }
    // }
    for(size_t it = i-1; it != -1; --it) { // i-1 é sempre >= 0
        for(int j=_base_bit_size_; j > 0; --j) {
            const auto bit = get_bit_int(_number_.at(it), j);
            const char c = bit != 0 ? '1' : '0';
            bnum = bnum + c;
        }
    }
    
    return bnum;
}

Div<bint, int> bint::div_by_10() const {
    
    if(zero() == true) return { bint(0), 0};
    
    /////////////////////////////////////////////////////////////////
    // realiza a primeira iteração da divisão
    /////////////////////////////////////////////////////////////////
    Iterator_Size_t pos (_virtual_number_size_, _virtual_number_size_ -1);
    const auto res1 = std::div(static_cast<int>(_number_.at(pos())), 10);
    decltype(_number_) quot;
    if(res1.quot != 0) { // o primeiro número do quociente não pode ser zero
        quot.push_back(static_cast<decltype(_number_.at(0))>(res1.quot)); // necessárimente menor que 256
    }
    int overflow = res1.rem;
    --pos;
    
    /////////////////////////////////////////////////////////////////
    // realiza as outras iterações da divisão
    /////////////////////////////////////////////////////////////////
    while(pos.end() == false) {
        /**
         * overflow << 8 => overflow*256
         * necessário para o cálculo do valor correto para a divisão na base 256.
         */
        
        int_least64_t dividend = static_cast<int_least64_t>(overflow);
        dividend = dividend << _base_bit_size_;
        dividend = dividend + static_cast<int_least64_t>(_number_.at(pos()));
        if(pos.end() == false) --pos; // atualiza pos -> pos > 0 == pos != 0
        
        if(dividend >= 10 && pos.end() == false) {
            for(int i = 0; i < 6 && pos.end() == false; ++i) {
                dividend = dividend << _base_bit_size_;
                dividend = dividend + static_cast<int_least64_t>(_number_.at(pos()));
                --pos;
            }
        }
        
        auto result = std::lldiv(dividend, 10);
        overflow = static_cast<int>(result.rem); // pega o resto
        
        // show("\nresult::quot: ",result.quot," | rem: ",result.rem,"\n");
        
        decltype(_number_) quot_part = {0, 0, 0, 0, 0, 0, 0};
        for(int i = 0; i < static_cast<int>(quot_part.size()); ++i) {
            quot_part.at(i) = result.quot & _base_mask_;
            result.quot = result.quot >> _base_bit_size_;
        }
        
        // show("quot_part::\n");
        // for(int i=0; i < quot_part.size(); ++i) {
        //     show("[",i,":",((int)quot_part.at(i)),"], ");
        // }
        // show("\n");
        
        int init = quot_part.size() -1;
        for(; init > -1; --init) {
            if(quot_part.at(init) != 0) break;
        }
        
        if(init == -1) {
            quot.push_back(0);
        } else {
            for(; init > -1; --init) {
                quot.push_back(quot_part.at(init));
            }
        }
    }
    
    /////////////////////////////////////////////////////////////////
    // cria o big_int que representa o quociente
    /////////////////////////////////////////////////////////////////
    bint bi_quot;
    if(quot.empty() || (quot.size() == 1 && quot.front() == 0)) { // bi_quot = 0
        bi_quot.set_zero();
    } 
    else { // bi_quot != 0
        bi_quot._virtual_number_size_ = 0;
        for(pos.set_reverse(quot.size()); pos.end() == false; --pos) {
            bi_quot._number_.push_back(quot.at(pos()));
            // bi_quot.push_back(quot.at(pos()));
        }
        bi_quot._is_negative_ = _is_negative_;
        bi_quot._is_zero_ = false;
        bi_quot._virtual_number_size_ = quot.size();
    }
    if(_is_negative_) overflow = overflow * -1;
    
    // show("\n=============================\nquot:\n");
    // show("quot.size(): ",quot.size(),"\n");
    // for(int i=0; i < quot.size(); ++i) { show("[",static_cast<int>(quot.at(i)),"], "); }
    // show("\n=============================\nbi_quot:sss\n");
    // bi_quot.print_in();
    // show("\n=============================\n");
    
    return {bi_quot, overflow};
}

bool bint::operator==(const bint& big_int) const {
    if(_is_negative_ != big_int._is_negative_) return false;
    if(_is_zero_ != big_int._is_zero_) return false;
    if(_virtual_number_size_ != big_int._virtual_number_size_) return false;
    
    for(size_t i=0; i < _virtual_number_size_; ++i) {
        if(_number_.at(i) != big_int._number_.at(i)) { return false; }
    }
    
    return true;
}

bool bint::operator<(const bint& big_int) const {
    // if(_is_negative_ == true && big_int._is_negative_ == false) return true;
    // if(_is_negative_ == false && big_int._is_negative_ == true) return false;
    // if(_is_zero_ == true && big_int._is_zero_ == false && big_int._is_negative_ == false) return true;
    // if(_is_zero_ == true && big_int._is_zero_ == false && big_int._is_negative_ == false) return true;
    // if(this->operator==(big_int) == true) return false;
    if(_is_negative_ == true) {
        if(big_int._is_negative_ == false) return true;
        
        /////////////////////////////////////////////////////////////////
        // ambos os número são negativos
        /////////////////////////////////////////////////////////////////
        if(_virtual_number_size_ > big_int._virtual_number_size_) return true;
        if(_virtual_number_size_ < big_int._virtual_number_size_) return false;
        
        /////////////////////////////////////////////////////////////////
        // ambos os número são do mesmo tamanho
        /////////////////////////////////////////////////////////////////
        for(size_t i=_virtual_number_size_; i > 0; --i) {
            if(_number_.at(i) > big_int._number_.at(i)) return true;
            else if(_number_.at(i) < big_int._number_.at(i)) return false;
            // else // se ambos os números são iguais naquela potência
        }
        // trata o caso de i == 0
        if(_number_.at(0) < big_int._number_.at(0)) return false;
        else return true;
    } else { // o número é positivo
        if(big_int._is_negative_ == true) return false;
        
        /////////////////////////////////////////////////////////////////
        // o número é zero
        /////////////////////////////////////////////////////////////////
        if(_is_zero_ == true) {
            if(big_int._is_zero_ == true) return false;
            else return true; // o número (big_int) é maior que o número de referencia
        }
        
        /////////////////////////////////////////////////////////////////
        // o número NÃO é zero
        /////////////////////////////////////////////////////////////////
        if(big_int._is_zero_ == true) return false;
        
        /////////////////////////////////////////////////////////////////
        // verifica o tamanho dos números
        /////////////////////////////////////////////////////////////////
        if(_virtual_number_size_ > big_int._virtual_number_size_) return false;
        if(_virtual_number_size_ < big_int._virtual_number_size_) return true;
        
        /////////////////////////////////////////////////////////////////
        // ambos os número são do mesmo tamanho
        /////////////////////////////////////////////////////////////////
        for(size_t i=_virtual_number_size_; i > 0; --i) {
            if(_number_.at(i) > big_int._number_.at(i)) return false;
            else if(_number_.at(i) < big_int._number_.at(i)) return true;
            // else // se ambos os números são iguais naquela potência
        }
        // trata o caso de i == 0
        if(_number_.at(0) < big_int._number_.at(0)) return true;
        else return false;
    }
}

bint bint::operator+(const bint& big_int) const {
    if(zero() == true) return big_int;
    if(big_int.zero() == true) return *this;
    
    /////////////////////////////////////////////////////////////////
    // ambos os número tem sinais iguais
    /////////////////////////////////////////////////////////////////
    if(_is_negative_ == big_int._is_negative_) {
        
        bint result (0);
        size_t equal_size;
        size_t bigger_size;
        decltype(_number_)* biggerp;
        
        if(_virtual_number_size_ > big_int._virtual_number_size_) {
            equal_size = big_int._virtual_number_size_;
            bigger_size = _virtual_number_size_;
            biggerp = const_cast<decltype(_number_)*>(&_number_);
        } else {
            equal_size = _virtual_number_size_;
            bigger_size = big_int._virtual_number_size_;
            biggerp = const_cast<decltype(_number_)*>(&big_int._number_);
        }
        
        
        int overflow = 0;
        size_t i;
        for(i=0; i < equal_size; ++i) {
            const int a = static_cast<int>(_number_.at(i));
            const int b = static_cast<int>(big_int._number_.at(i));
            const int res = a + b + overflow;
            const int r1 = res & 255; // valor desta casa
            overflow = res >> 8; // armazena o valor que será colocado na próxima casa da operação
            result.push_back(r1);
        }
        
        // somente quando ainda tem um overflow != 0
        const auto& bigger = *biggerp;
        for(; i < bigger_size; ++i) {
            const int r = static_cast<int>(bigger.at(i));
            const int res = r + overflow;
            const int r1 = res & 255; // valor desta casa
            overflow = res >> 8; // armazena o valor que será colocado na próxima casa da operação
            result.push_back(r1);
            if(overflow == 0) {
                ++i;
                break;
            }
        }
        
        // otimização para melhor a performance, não precisa de realizar outras operações
        for(; i < bigger_size; ++i) {
            result.push_back(bigger.at(i));
        }
        
        // para o caso de _virtual_number_size_ == big_int._virtual_number_size_ e ter um overflow != 0
        while(overflow != 0) {
            result.push_back(overflow);
            overflow = overflow >> 8;
            // if(overflow >= _base_) { // não precisa pois é garantido que overflow < 256.
            //     show("ERROR - overflow in sum cannot be equal or bigger than base number. base: ",_base_,", overflow: ",overflow,"\n");
                
            // }
        }
        
        result._is_negative_ = _is_negative_;
        result._is_zero_ = false;
        return result;
    } else {
    /////////////////////////////////////////////////////////////////
    // trata o caso quando os números tem sinais diferentes
    /////////////////////////////////////////////////////////////////
        switch(this->cmp_abs(big_int)) {
            case 0:  return {0};
            case 1:  return private_subtration(*this, big_int);
            case -1: return private_subtration(big_int, *this);
            default:
                throw binterror("value of cmp_abs is undefined - not{-1, 1, 0}");
                // show("ERROR - LINE: ",__LINE__,"\n");
                // abort();
        }
    }
    
    return {};
}

bint bint::operator-(bint& big_int) const {
    if(big_int.zero()) {
        return *this;
    }
    big_int._is_negative_ = !big_int._is_negative_;
    bint result = this->operator+(big_int);
    big_int._is_negative_ = !big_int._is_negative_;
    
    return result;
}

bint bint::operator*(const bint& big_int) const {
    if(zero() || big_int.zero()) {
        return 0;
    }
    
    bint result = 0;
    const auto rsize = _virtual_number_size_ + big_int._virtual_number_size_ +1;
    if(rsize > 256) { // por default o tamanho é 256
        result._number_.resize(rsize, 0);
    } else if(result._number_.empty()) {
        result._number_.resize(256, 0);
    }
    
    int overflow;
    for(size_t i=0; i < big_int._virtual_number_size_; ++i) {
        const int a = static_cast<int>(big_int._number_.at(i));
        if(a == 0) continue;
        
        overflow = 0;
        size_t j=0;
        for(; j < _virtual_number_size_; ++j) {
            const int r1 = a * static_cast<int>(_number_.at(j));
            const int r2 = r1 + overflow;
            // if(i+j >= result._number_.size()) {
            //     result._number_.resize(i+j+1, 0);
            // }
            const int r3 = r2 + static_cast<int>(result._number_.at(i+j));
            const int r4 = r3 & _base_mask_;
            overflow     = r3 >> _base_bit_size_;
            result._number_.at(i+j) = static_cast<unsigned char>(r4);
        }
        
        while(overflow > 0) {
            // if(i+j >= result._number_.size()) {
            //     result._number_.resize(i+j+1, 0);
            // }
            const int r1 = overflow + static_cast<int>(result._number_.at(i+j));
            const int r2 = r1 & _base_mask_; // unsigned char = 255
            overflow     = r1 >> _base_bit_size_; // unsigned char = 8
            result._number_.at(i+j) = static_cast<unsigned char>(r2);
            ++j;
        }
    }
    
    result.update_virtual_number_size();
    result._is_negative_ = _is_negative_ xor big_int._is_negative_;
    result._is_zero_ = false;
    return result;
}

/*
bint& bint::operator=(const bint& number) {
    if(number._is_zero_ == true) {
        set_zero();
        return *this;
    }
    
    // Guard self assignment
    if(this == &number) return *this;
    
    _number_ = number._number_;
    _virtual_number_size_ = number._virtual_number_size_;
    _is_negative_ = number._is_negative_;
    _is_zero_ = number._is_zero_;
    
    return *this;
} */

Div<bint, bint> bint::div1(const bint& big_int) const {
    if(big_int.zero()) {
        throw binterror("divisor cannot be zero.");
        // show("ERROR - divisor cannot be zero. - line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = 0;
        return result;
    }
    
    if(big_int == 1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.rem = 0;
        return result;
    }
    
    if(big_int == -1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.quot._is_negative_ = !big_int._is_negative_;
        result.rem = 0;
        return result;
    }
    
    if(*this == big_int) {
        Div<bint, bint> result;
        result.quot = 1;
        result.rem = 0;
        return result;
    }
    if(this->cmp_abs(big_int) == 0) {
        Div<bint, bint> result;
        result.quot = -1;
        result.rem = 0;
        return result;
    }
    
    if(this->cmp_abs(big_int) == -1) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = *this;
        return result;
    }
    
    bint remainder = this->abs();
    bint quotient ("0");
    const bint one ("1");
    const auto& divisor = big_int;
    
    do {
        if(divisor.negative()) {
            remainder.add(divisor);
        } else {
            remainder.sub(divisor);
        }
        quotient.add(one);
        
        if(remainder.cmp_abs(big_int) == 0) {
            remainder.set_zero();
            quotient.add(one);
            break;

        } else if(remainder.cmp_abs(big_int) == -1) {
            break;
        }
    } while(true);
    
    quotient._is_negative_ = this->_is_negative_ xor big_int._is_negative_ ? true : false;
    if(remainder.zero() == false) {
        remainder._is_negative_ = _is_negative_;
    }
    
    return {quotient, remainder};
}

Div<bint, bint> bint::div2(const bint& big_int) const {
    if(big_int.zero()) {
        throw binterror("divisor cannot be zero.");
        // show("ERROR - divisor cannot be zero. - line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = 0;
        return result;
    }
    
    if(big_int == 1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.rem = 0;
        return result;
    }

    if(big_int == -1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.quot._is_negative_ = !_is_negative_;
        result.rem = 0;
        return result;
    }
    
    if(*this == big_int) {
        Div<bint, bint> result;
        result.quot = 1;
        result.rem = 0;
        return result;
    }
    if(this->cmp_abs(big_int) == 0) {
        Div<bint, bint> result;
        result.quot = -1;
        result.rem = 0;
        return result;
    }
    
    if(this->cmp_abs(big_int) == -1) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = *this;
        return result;
    }
    
    auto remainder = this->abs();
    bint quotient = 0;
    bint quot = 1;
    auto divisor = big_int.abs();
    const int divisor_most_significant_bit = big_int.get_most_significat_bit_pos();
    const long long dividor_bits = divisor_most_significant_bit + (big_int._virtual_number_size_ -1)*_base_bit_size_;
    do {
        const int dividend_most_significant_bit = remainder.get_most_significat_bit_pos();
        const long long dividend_bits = dividend_most_significant_bit + (remainder._virtual_number_size_ -1)*_base_bit_size_;
        long long bit_diff = dividend_bits - dividor_bits;
        divisor.shift_left(bit_diff); // auto divisor = big_int << bit_diff;

        const auto diff2 = remainder.cmp_abs(divisor);
        switch(diff2) {
            case 0: // otimização
                remainder.set_zero();
                quot.shift_left(bit_diff); // const auto quot = one << bit_diff;
                quotient.add(quot);
                goto FINISH_DIVISION; // quebra o loop
            case -1:
                divisor.shift_r1();
                bit_diff = bit_diff -1;
                break;
        }

        quot.shift_left(bit_diff); // const auto quot = one << bit_diff;
        quotient.add(quot);
        remainder.sub(divisor);

        // otimização: é utilizado o cmp_abs(big_int) para se realizar 1 divisor.shift_right() a menos.
        const auto diff = remainder.cmp_abs(big_int);
        switch (diff) {
            case 1:
                divisor.shift_right(bit_diff); // volta o divisor para big_int.abs()
                quot.shift_right(bit_diff); // volta o valor para 1
                break;

            case 0: 
                    remainder.set_zero(); 
                    goto FINISH_DIVISION; // quebra o loop -> para ficar mais legível >> esse goto não é necessário
            case -1: goto FINISH_DIVISION; // quebra o loop
            default:
                throw binterror("absolute comparation result: ",diff);
                // show("ERROR - absolute comparation result: ",diff," - line: ",__LINE__,"\n");
                // exit(EXIT_FAILURE);
        }
    } while(true);
    FINISH_DIVISION:

    quotient._is_negative_ = _is_negative_ xor big_int._is_negative_ ? true : false;
    if(remainder.zero() == false) {
        remainder._is_negative_ = _is_negative_;
    }
    
    Div<bint, bint> result;
    result.quot = quotient;
    result.rem = remainder;
    return result;
}

int bint::get_most_significat_bit_pos() const {
    if(zero()) {
        return 0;
    }

    const static std::vector<unsigned int> mask = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648 };

    const unsigned int a = static_cast<unsigned int>(_number_.at(_virtual_number_size_-1));
    for(int i=_base_bit_size_; i > 0; --i) {
        if((a & mask.at(i)) != 0) {
            return i;
        }
    }

    throw binterror("the most significant bit cannot be in position zero. - number is not zero. - number_chunck: ",a,", number: ",str());
    // show("ERROR - the most significant bit cannot be in position zero. - number is not zero. - number_chunck: ",a,", number: ",str(),", line: ",__LINE__,"\n");
    // exit(EXIT_FAILURE);
    return 0;
}

Div<bint, bint> bint::div3(const bint& big_int) const {
    if(big_int.zero()) {
        throw binterror("divisor cannot be zero.");
        // show("ERROR - divisor cannot be zero. - line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = 0;
        return result;
    }
    
    if(big_int == 1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.rem = 0;
        return result;
    }

    if(big_int == -1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.quot._is_negative_ = !_is_negative_;
        result.rem = 0;
        return result;
    }
    
    if(*this == big_int) {
        Div<bint, bint> result;
        result.quot = 1;
        result.rem = 0;
        return result;
    }
    if(this->cmp_abs(big_int) == 0) {
        Div<bint, bint> result;
        result.quot = -1;
        result.rem = 0;
        return result;
    }
    
    if(this->cmp_abs(big_int) == -1) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = *this;
        return result;
    }
    
    auto remainder = this->abs();
    bint quotient = 1;
    auto divisor = big_int.abs();
    const auto divisor_most_significat_bit_pos = divisor.get_most_significat_bit_pos();
    size_t begin_old = 0;
    int bit_diff_old = 0;
    bool first_iteration = true;
    do {
        // show("\n#################################\n");
        const auto remainder_most_significat_bit_pos = remainder.get_most_significat_bit_pos();
        auto bit_diff = remainder_most_significat_bit_pos - divisor_most_significat_bit_pos;
        if(bit_diff < 0) {
            bit_diff = bit_diff + _base_bit_size_;
        }

        if(divisor != big_int.abs()) {
            // show("ERROR - \n");
            // show("big_int: ",big_int.bstr(),"\n");
            // show("divisor: ",divisor.bstr(),"\n");
            exit(1);
        }
        divisor.shift_left(bit_diff);
        auto begin = remainder._virtual_number_size_ - divisor._virtual_number_size_;
        const auto diff = remainder.cmp_for_div(divisor, bit_diff);
        // show("remainder: ",remainder.bstr(),"\n");
        switch(diff) {
            case  1:
                // show("case 1:\n");
                remainder.private_sub_self_minuend_for_div(divisor, begin);
                break;
            
            case  0: {
                // show("case 0:\n");
                remainder.private_sub_self_minuend_for_div(divisor, begin);
                if(remainder.zero()) {
                    if(first_iteration == false) {
                        quotient.shift_l1();
                        quotient.add(1);
                    }
                    const long long num0s = begin*_base_bit_size_ + bit_diff;
                    quotient.shift_left(num0s);
                    goto FINISH_DIVISON;
                }
                break;

            } case -1:
                // show("\ncase -1:\n");
                if(bit_diff == 0) {
                    // show("bit_diff == 0\n");
                    begin = begin -1; // sempre deve ser >= 1 (antes da subtração), pois remainder > divisor
                    bit_diff = _base_bit_size_ -1;
                    divisor.shift_left(bit_diff);
                    remainder.private_sub_self_minuend_for_div(divisor, begin);
                } else {
                    // show("bit_diff == ",bit_diff,"\n");
                    bit_diff = bit_diff -1;
                    divisor.shift_r1();
                    remainder.private_sub_self_minuend_for_div(divisor, begin);
                }
        }

        
        long long num0s = 0;
        if(first_iteration == true) {} 
        else {
            num0s = (begin_old - begin)*_base_bit_size_ + bit_diff_old - bit_diff;
        }

        // show("begin_old: ",begin_old,"\n");
        // show("bit_diff_old: ",bit_diff_old,"\n");
        // show("begin: ",begin,"\n");
        // show("bit_diff: ",bit_diff,"\n");
        // show("num0s: ",num0s,"\n");
        // show("[x]quotient: ",quotient.bstr(),"\n");
        // show("remainder: ",remainder.bstr(),"\n");
        // show("remainder: ",remainder,"\n");
        const auto cmp = remainder.cmp_abs(big_int);
        switch(cmp) {
            case  1:
                // show("cmp01:\n");
                if(first_iteration == true) {
                    first_iteration = false;
                } else {
                    num0s = num0s -1;
                    quotient.shift_left(num0s);
                    quotient.shift_l1();
                    quotient.add(1);
                }
                divisor.shift_right(bit_diff);
                begin_old = begin;
                bit_diff_old = bit_diff;
                // show("[1]quotient: ",quotient.bstr(),"\n");
                break;
            case  0:
                // show("cmp00:\n");
                // if(first_iteration == true) {
                //     num0s = 
                // } else {
                //     num0s = begin*_base_bit_size_ +bit_diff -1; 
                // }
                num0s = begin*_base_bit_size_ +bit_diff -1;
                quotient.shift_left(num0s);
                quotient.shift_l1();
                quotient.add(1);
                remainder.set_zero();
                // show("[2]quotient: ",quotient.bstr(),"\n");
                goto FINISH_DIVISON; // end_division = true; break;
            case -1:
                // show("cmp-1:\n");
                if(first_iteration == true) {
                    num0s = begin*_base_bit_size_ + bit_diff;
                
                } else {
                    num0s = num0s -1;
                    quotient.shift_left(num0s);
                    quotient.shift_l1();
                    quotient.add(1);

                    num0s = begin*_base_bit_size_ + bit_diff;
                    // if(begin == 0 && bit_diff == 0) {
                    //     num0s = 0;
                    // } else {
                    //     const auto r = remainder._virtual_number_size_ -1;
                    //     if(r > begin) {
                    //         num0s = (remainder._virtual_number_size_ -begin)*_base_bit_size_ +remainder.get_most_significat_bit_pos() -bit_diff;

                    //     } else if(r < begin) {
                    //             num0s = (begin -remainder._virtual_number_size_)*_base_bit_size_ +bit_diff -remainder.get_most_significat_bit_pos();

                    //     } else { // r == begin
                    //             num0s = std::abs(remainder.get_most_significat_bit_pos() -bit_diff);
                    //     }
                    // }
                }
                // show("remainder.get_most_significat_bit_pos(): ",remainder.get_most_significat_bit_pos(),"\n");
                // show("remainder._virtual_number_size_: ",remainder._virtual_number_size_,"\n");
                // show("num0s: ",num0s,"\n");
                quotient.shift_left(num0s);
                // show("[-]quotient: ",quotient.bstr(),"\n");
                goto FINISH_DIVISON; // end_division = true; break;
        }
    } while(true);
    
    FINISH_DIVISON:
    quotient._is_negative_ = _is_negative_ xor big_int._is_negative_ ? true : false;
    if(remainder.zero() == false) {
        remainder._is_negative_ = _is_negative_;
    }

    Div<bint, bint> result;
    result.quot = quotient;
    result.rem  = remainder;
    return result;
}

int bint::cmp_for_div(const bint& divisor, const int bit_shift_right) const {
    for(size_t i = 0; i < divisor._virtual_number_size_-1; ++i) {
        const auto rem_a = static_cast<int>(_number_.at(_virtual_number_size_ -1 -i));
        const auto div_a = static_cast<int>(divisor._number_.at(divisor._virtual_number_size_ -1 -i));
        const auto diff = rem_a - div_a;
        if(diff > 0) {
            return 1;
        } else if(diff < 0) {
            return -1;
        }
    }

    // faz a primeira operação
    auto rem_a = static_cast<int>(_number_.at(_virtual_number_size_ -divisor._virtual_number_size_));
    rem_a = rem_a >> bit_shift_right;
    auto div_a = static_cast<int>(divisor._number_.at(0));
    div_a = div_a >> bit_shift_right;
    const auto diff = rem_a - div_a;
    if(diff > 0) {
        return 1;
    } else if(diff < 0) {
        return -1;
    }

    return 0;
}

Div<bint, bint> bint::div4(const bint& big_int) const {
    if(big_int.zero()) {
        throw binterror("divisor cannot be zero.");
        // show("ERROR - divisor cannot be zero. - line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = 0;
        return result;
    }
    
    if(big_int == 1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.rem = 0;
        return result;
    }

    if(big_int == -1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.quot._is_negative_ = !_is_negative_;
        result.rem = 0;
        return result;
    }
    
    if(*this == big_int) {
        Div<bint, bint> result;
        result.quot = 1;
        result.rem = 0;
        return result;
    }
    if(this->cmp_abs(big_int) == 0) {
        Div<bint, bint> result;
        result.quot = -1;
        result.rem = 0;
        return result;
    }
    
    if(this->cmp_abs(big_int) == -1) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = *this;
        return result;
    }

    const auto divisor = big_int.abs();
    const auto divisor_size = divisor._virtual_number_size_ -1;
    const auto divisor_most_signficant_bit_pos = divisor.get_most_significat_bit_pos();
    const auto divisor_bits_total = static_cast<long long>(divisor_size)*_base_bit_size_ +divisor_most_signficant_bit_pos; 
    auto dividend_size_pos = this->_virtual_number_size_ -1;
    auto dividend_bit_pos = this->get_most_significat_bit_pos();
    bint quotient = 0;
    bint remainder = 0;
    bool end_division = false;
    do {
        auto numbits = remainder.div_fill(divisor, divisor_bits_total, 
            *this, dividend_size_pos, dividend_bit_pos);
        if(numbits > 0) {
            bool continue_div = remainder >= divisor ? true : false;
            if(dividend_size_pos != 0 || dividend_bit_pos != 0 || continue_div == true) {
                numbits = numbits -1;
            }
            quotient.shift_left(numbits);
            
            if(continue_div == true) {
                remainder.sub(divisor);
                quotient.shift_l1();
                if(quotient.zero() == true) {
                    if(quotient._number_.empty() == true) {
                        quotient._number_.resize(256, 0);
                    }
                    quotient._is_zero_ = false;
                    quotient._virtual_number_size_ = 1;
                }
                quotient._number_.at(0) |= 1; // = quotient.add(1); // otimização
            }
        } else {
            end_division = true;
        }
    } while(end_division == false);

    quotient._is_negative_ = _is_negative_ xor big_int._is_negative_ ? true : false;
    if(remainder.zero() == false) {
        remainder._is_negative_ = _is_negative_;
    }

    Div<bint, bint> result;
    result.quot = quotient;
    result.rem  = remainder;
    return result;
}

Div<bint, bint> bint::div5(const bint& big_int) const {
    if(big_int.zero()) {
        throw binterror("divisor cannot be zero");
        // show("ERROR - divisor cannot be zero. - line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = 0;
        return result;
    }
    
    if(big_int == 1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.rem = 0;
        return result;
    }

    if(big_int == -1) {
        Div<bint, bint> result;
        result.quot = *this;
        result.quot._is_negative_ = !_is_negative_;
        result.rem = 0;
        return result;
    }
    
    if(*this == big_int) {
        Div<bint, bint> result;
        result.quot = 1;
        result.rem = 0;
        return result;
    }
    if(this->cmp_abs(big_int) == 0) {
        Div<bint, bint> result;
        result.quot = -1;
        result.rem = 0;
        return result;
    }
    
    if(this->cmp_abs(big_int) == -1) {
        Div<bint, bint> result;
        result.quot = 0;
        result.rem = *this;
        return result;
    }

    const auto divisor = big_int.abs();
    const auto divisor_size = divisor._virtual_number_size_ -1;
    const auto divisor_most_signficant_bit_pos = divisor.get_most_significat_bit_pos();
    const auto divisor_bits_total = static_cast<long long>(divisor_size)*_base_bit_size_ +divisor_most_signficant_bit_pos; 
    auto dividend_size_pos = this->_virtual_number_size_ -1;
    auto dividend_bit_pos = this->get_most_significat_bit_pos();
    bint quotient = 0;
    bint remainder = 0;
    bool end_division = false;
    do {
        auto numbits = remainder.div_fill2(divisor, divisor_bits_total, 
            *this, dividend_size_pos, dividend_bit_pos);
        if(numbits > 0) {
            bool continue_div = remainder >= divisor ? true : false;
            if(dividend_size_pos != 0 || dividend_bit_pos != 0 || continue_div == true) {
                numbits = numbits -1;
            }
            quotient.shift_left(numbits);
            
            if(continue_div == true) {
                remainder.sub(divisor);
                quotient.shift_l1();
                if(quotient.zero() == true) {
                    if(quotient._number_.empty() == true) {
                        quotient._number_.resize(256, 0);
                    }
                    quotient._is_zero_ = false;
                    quotient._virtual_number_size_ = 1;
                }
                quotient._number_.at(0) |= 1; // = quotient.add(1); // otimização
            }
        } else {
            end_division = true;
        }
    } while(end_division == false);

    quotient._is_negative_ = _is_negative_ xor big_int._is_negative_ ? true : false;
    if(remainder.zero() == false) {
        remainder._is_negative_ = _is_negative_;
    }

    Div<bint, bint> result;
    result.quot = quotient;
    result.rem  = remainder;
    return result;
}

size_t bint::div_fill(const bint& divisor, const size_t divisor_size, 
    const int divisor_most_signficant_bit_pos, const bint& dividend, 
    size_t& dividend_size_pos, int& dividend_bit_pos) {

        if(dividend_size_pos == 0 && dividend_bit_pos == 0) {
            return 0;
        }

        // show("INIT - dividend_size_pos: ",dividend_size_pos,", dividend_bit_pos: ",dividend_bit_pos,", rem: ",str(),", line: ",__LINE__,"\n");
        size_t bit0sl = 0;
        if(zero()) {
            bool end = false;
            for(; dividend_size_pos != -1 && end == false; ++bit0sl) {
                unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
                const auto bit = get_bit(block, dividend_bit_pos);
                if(bit == 1) {
                    add(1);
                    end = true;
                } // else{} // se for zero não precisa fazer nada

                dividend_bit_pos = dividend_bit_pos -1;
                if(dividend_bit_pos == 0) {
                    dividend_size_pos = dividend_size_pos -1; // atualiza
                    dividend_bit_pos = _base_bit_size_;
                }
            }
        }

        const auto divisor_bits_total = divisor_size*_base_bit_size_ + divisor_most_signficant_bit_pos;
        
        const auto remainder_bits_total = zero() == true ? 0 : (_virtual_number_size_ -1)*_base_bit_size_ + get_most_significat_bit_pos();
        const auto diff_bits = divisor_bits_total - remainder_bits_total;

        size_t i = 0;
        for(;i < diff_bits && dividend_size_pos != -1; ++i) {

                shift_l1();
                unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
                const auto bit = get_bit(block, dividend_bit_pos);
                if(bit == 1) {
                    add(1);
                } // else{} // se for zero não precisa fazer nada

                dividend_bit_pos = dividend_bit_pos -1;
                if(dividend_bit_pos == 0) {
                    dividend_size_pos = dividend_size_pos -1; // atualiza
                    dividend_bit_pos = _base_bit_size_;
                }
        }

        if(*this < divisor && dividend_size_pos != -1) {
            shift_l1();
            unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
            const auto bit = get_bit(block, dividend_bit_pos);
            if(bit == 1) {
                add(1);
            } // else{} // se for zero não precisa fazer nada

            dividend_bit_pos = dividend_bit_pos -1;
            if(dividend_bit_pos == 0) {
                dividend_size_pos = dividend_size_pos -1; // atualiza
                dividend_bit_pos = _base_bit_size_;
            }
            i = i+1; // atualiza o número de bits
        }

        if(dividend_size_pos == -1) { // trata a saída
            dividend_size_pos = 0;
            dividend_bit_pos = 0;
        }

        return i+bit0sl;
}

size_t bint::div_fill(const bint& divisor, const long long divisor_bits_total,
    const bint& dividend, size_t& dividend_size_pos, int& dividend_bit_pos) {

        if(dividend_size_pos == 0 && dividend_bit_pos == 0) {
            return 0;
        }

        size_t bit0sl = 0;
        if(zero()) {
            bool end = false;
            for(; dividend_size_pos != -1 && end == false; ++bit0sl) {
                unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
                const auto bit = get_bit_int(block, dividend_bit_pos);
                if(bit != 0) {
                    // add(1); // otimização abaixo para substituir a função add(1)
                    if(_number_.empty() == true) {
                        _number_.resize(256, 0);
                    }
                    _number_.at(0) = 1;
                    _virtual_number_size_ = 1;
                    _is_zero_ = false;
                    end = true;
                } // else{} // se for zero não precisa fazer nada

                dividend_bit_pos = dividend_bit_pos -1;
                if(dividend_bit_pos == 0) {
                    dividend_size_pos = dividend_size_pos -1; // atualiza
                    dividend_bit_pos = _base_bit_size_;
                }
            }
        }
        
        const auto remainder_bits_total = zero() == true ? 0 : (_virtual_number_size_ -1)*_base_bit_size_ + get_most_significat_bit_pos();
        const auto diff_bits = divisor_bits_total - remainder_bits_total;
        size_t i = 0;
        for(;i < diff_bits && dividend_size_pos != -1; ++i) {

                shift_l1();
                unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
                const auto bit = get_bit_int(block, dividend_bit_pos);
                if(bit != 0) {
                    _number_.at(0) |= 1; // = add(1) -> otimização, por causa do shift_l1() acima.
                } // else{} // se for zero não precisa fazer nada

                dividend_bit_pos = dividend_bit_pos -1;
                if(dividend_bit_pos == 0) {
                    dividend_size_pos = dividend_size_pos -1; // atualiza
                    dividend_bit_pos = _base_bit_size_;
                }
        }

        if(*this < divisor && dividend_size_pos != -1) {
            shift_l1();
            unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
            const auto bit = get_bit_int(block, dividend_bit_pos);
            if(bit != 0) {
                _number_.at(0) |= 1; // = add(1) -> otimização, por causa do shift_l1() acima.
            } // else{} // se for zero não precisa fazer nada

            dividend_bit_pos = dividend_bit_pos -1;
            if(dividend_bit_pos == 0) {
                dividend_size_pos = dividend_size_pos -1; // atualiza
                dividend_bit_pos = _base_bit_size_;
            }
            i = i+1; // atualiza o número de bits
        }

        if(dividend_size_pos == -1) { // trata a saída
            dividend_size_pos = 0;
            dividend_bit_pos = 0;
        }

        return i+bit0sl;
}

size_t bint::div_fill2(const bint& divisor, const long long divisor_bits_total,
    const bint& dividend, size_t& dividend_size_pos, int& dividend_bit_pos) {

        if(dividend_size_pos == 0 && dividend_bit_pos == 0) {
            return 0;
        }

        size_t bit0sl = 0; // numero de bits 0 a esquerda o primeiro 1 no quociente
        if(zero()) {
            bool end = false;
            for(; dividend_size_pos != -1 && end == false; ++bit0sl) {
                unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
                const auto bit = get_bit_int(block, dividend_bit_pos);
                if(bit != 0) {
                    // add(1);
                    if(_number_.empty() == true) {
                        _number_.resize(256, 0);
                    }
                    _number_.at(0) = 1;
                    _virtual_number_size_ = 1;
                    _is_zero_ = false;
                    end = true;
                } // else{} // se for zero não precisa fazer nada

                dividend_bit_pos = dividend_bit_pos -1;
                if(dividend_bit_pos == 0) {
                    dividend_size_pos = dividend_size_pos -1; // atualiza
                    dividend_bit_pos = _base_bit_size_;
                }
            }
        }
        
        const auto remainder_bits_total = zero() == true ? 0 : (_virtual_number_size_ -1)*_base_bit_size_ +get_most_significat_bit_pos();
        const auto diff_bits = divisor_bits_total - remainder_bits_total;
        const auto dividend_bits_total = dividend_size_pos*_base_bit_size_ + dividend_bit_pos;

        auto shift_size = diff_bits < dividend_bits_total ? diff_bits : dividend_bits_total;
        shift_left(static_cast<long long>(shift_size));

        const static std::vector<unsigned int> mask = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648 };
        
        const auto x = std::lldiv(shift_size, _base_bit_size_);
        auto remainder_block_id = x.rem == 0 ? x.quot -1 : x.quot; // atualiza posição do bloco
        auto bit_pos = x.rem == 0 ? _base_bit_size_ : x.rem;
        size_t i = 0;
        for(;i < diff_bits && dividend_size_pos != -1; ++i) {
            const auto block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
            const auto bit = get_bit_int(block, dividend_bit_pos);
            if(bit != 0) { // insere 1 na posição
                _number_.at(remainder_block_id) |= mask.at(bit_pos);
            } // else{} // se for zero não precisa fazer nada

            dividend_bit_pos = dividend_bit_pos -1;
            if(dividend_bit_pos == 0) {
                dividend_size_pos = dividend_size_pos -1; // atualiza
                dividend_bit_pos = _base_bit_size_;
            }

            bit_pos = bit_pos -1; // atualiza o id do bit do remainder
            if(bit_pos == 0) {
                bit_pos = _base_bit_size_;
                remainder_block_id = remainder_block_id -1; // atualiza o bloco do bit do remainder
            }
        }

        if(*this < divisor && dividend_size_pos != -1) {
            shift_l1();
            const unsigned int block = static_cast<unsigned int>(dividend._number_.at(dividend_size_pos));
            const auto bit = get_bit_int(block, dividend_bit_pos);
            if(bit != 0) {
                _number_.at(0) |= 1; // = add(1) -> otimização, por causa do shift_l1() acima.
            } // else{} // se for zero não precisa fazer nada

            dividend_bit_pos = dividend_bit_pos -1;
            if(dividend_bit_pos == 0) {
                dividend_size_pos = dividend_size_pos -1; // atualiza
                dividend_bit_pos = _base_bit_size_;
            }
            i = i+1; // atualiza o número de bits
        }

        if(dividend_size_pos == -1) { // trata a saída
            dividend_size_pos = 0;
            dividend_bit_pos = 0;
        }

        return i+bit0sl;
}

int bint::get_bit(const unsigned int block, const int pos) const {
    if(pos > 32) {
        throw binterror("position is greater than 32. pos: ",pos);
        // show("ERROR - position is greater than 32. pos: ",pos,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(pos < 1) {
        throw binterror("position is smaller than 1. - Init position is 1. pos: ",pos);
        // show("ERROR - position is smaller than 1. - Init position is 1. pos: ",pos,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }

    const static std::vector<unsigned int> mask = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648 };

    const int bit = (block & mask.at(pos)) == 0 ? 0 : 1;
    return bit;
}

unsigned int bint::get_bit_int(const unsigned int block, const int pos) const {
    if(pos > 32) {
        throw binterror("position is greater than 32. pos: ",pos);
        // show("ERROR - position is greater than 32. pos: ",pos,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(pos < 1) {
        throw binterror("position is smaller than 1. - Init position is 1. pos: ",pos);
        // show("ERROR - position is smaller than 1. - Init position is 1. pos: ",pos,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }

    const static std::vector<unsigned int> mask = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648 };

    const auto bit = block & mask.at(pos);
    return bit;
}

bint bint::pow1(const long long power) const {
    if(zero()) {
        return {0};
    }
    if(power == 0) {
        return {1};
    }
    if(power < 1) {
        throw binterror("power must be greater or equal 1. power: ",power);
        // show("ERROR - power must be greater or equal 1. line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(power == 1) {
        return *this;
    }
    
    std::map<long long, bint> history;
    history.emplace(1, *this);
    
    ////////////////////////////////////////////////////////////////
    // cria um histórico de potências do número
    ////////////////////////////////////////////////////////////////
    bint& num = *const_cast<bint*>(this);
    long long i = 1;
    do {
        // show("<<::i: ",i,", power: ",power,"\n");
        const auto ni = i << 1; // multiplica por dois a pontência
        if(ni <= i) { // check for error
            // show("ERROR - long long is not enough to make this calculations. Utilize other type to pass the power for this function: ex: bint or std::string. Line: ",__LINE__,"\n");
            // exit(EXIT_FAILURE);
            // TODO - melhorar isso depois, pois desse jeito não está eficiente
            bint p = power;
            return pow1(p); // realiza como big_int a operação
        }
        i = ni;

        if(i == power) {
            return num*num;
        } else if(i > power) {
            i = i >> 1; // volta ao valor anterior
            break;
        }
        
        num = num * num;
        history.emplace(i, num);
    } while(true);
    
    ////////////////////////////////////////////////////////////////
    // procura o atual valor da potência, quando a potência não for uma potência de 2.
    ////////////////////////////////////////////////////////////////
    do {
        const auto diff = power - i;
        // show("i: ",i,", power: ",power,", diff: ",diff,", num.size(): ",num._number_.size(),", num.vs: ",num._virtual_number_size_,"\n");
        
        auto rit = history.rbegin();
        ++rit; // o primeiro item sempre é maior que diff
        for (; rit != history.rend(); ++rit) {
            if(rit->first < diff) {
                num = num * rit->second;
                i = i + rit->first;
                break;
                
            } else if(rit->first == diff) {
                // if(diff == 1 && rit->second == 71) {
                    // show("rit->second: ",rit->second,"\n");
                    // show("==============\n");
                    num = num * rit->second;
                    // show("after mum:;==============\n");
                    // show(num,"\n");
                    // show("top her?\n");
                // }
                return num;
            }
        }
    } while(true);
    
    throw binterror("can NOT be get here.");
    // show("ERROR - line: ",__LINE__,"\n");
    // exit(EXIT_FAILURE);
    return {0};
}

bint bint::pow1(const bint& power) const {
    if(zero()) {
        return {0};
    }
    if(power.zero()) {
        return {1};
    }
    if(power < 1) {
        throw binterror("power must be greater or equal 1. power: ",power);
        // show("ERROR - power must be greater or equal 1. line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(power == 1) {
        return *this;
    }
    
    std::map<bint, bint> history;
    history.emplace(bint(1), *this);
    
    ////////////////////////////////////////////////////////////////
    // cria um histórico de potências do número
    ////////////////////////////////////////////////////////////////
    bint& num = *const_cast<bint*>(this);
    bint i {1};
    do {
        // show("<<::i: ",i,", power: ",power,"\n");
        // i = i << 1; // multiplica por dois a pontência
        i.shift_l1(); // multiplica por dois a pontência
        if(i == power) {
            return num*num;
        } else if(i > power) {
            // i = i >> 1; // volta ao valor anterior
            i.shift_r1(); // volta ao valor anterior
            break;
        }
        
        num = num * num;
        history.emplace(i, num);
    } while(true);
    
    ////////////////////////////////////////////////////////////////
    // procura o atual valor da potência, quando a potência não for uma potência de 2.
    ////////////////////////////////////////////////////////////////
    auto diff = power - i;
    do {
        // show("i: ",i,", power: ",power,", diff: ",diff,", num.size(): ",num._number_.size(),", num.vs: ",num._virtual_number_size_,"\n");
        
        auto rit = history.rbegin();
        ++rit; // o primeiro item sempre é maior que diff
        for(; rit != history.rend(); ++rit) {
            if(rit->first < diff) {
                num = num * rit->second;
                diff.sub(rit->first); // atualiza o diff para a próxima iteração
                break;
                
            } else if(rit->first == diff) {
                // if(diff == 1 && rit->second == 71) {
                    // show("rit->second: ",rit->second,"\n");
                    // show("==============\n");
                    num = num * rit->second;
                    // show("after mum:;==============\n");
                    // show(num,"\n");
                    // show("top her?\n");
                // }
                return num;
            }
        }
    } while(true);
    
    throw binterror("can NOT get here.");
    // show("ERROR - line: ",__LINE__,"\n");
    // exit(EXIT_FAILURE);
    return {0};
}

///////////////////////////////////////////////////////////////////////////////////////
// operations operator binary functions >> << and or xor not
///////////////////////////////////////////////////////////////////////////////////////
bint bint::operator&(const bint& big_int) const {
    bint result (0);
    if(zero() || big_int.zero()) {
        return result;
    }
    
    // escolhe o menor valor
    size_t result_size = _virtual_number_size_ > big_int._virtual_number_size_ ? big_int._virtual_number_size_ : _virtual_number_size_;
    
    for(size_t i=0; i < result_size; ++i) {
        const auto a = _number_.at(i);
        const auto b = big_int._number_.at(i);
        const auto r = a & b;
        result.push_back(r);
    }
    
    result.update_virtual_number_size(result_size);
    if(result.zero() == false) {
        result._is_negative_ = _is_negative_ and big_int._is_negative_;
    }
    
    return result;
}

bint bint::operator|(const bint& big_int) const {
    if(zero()) {
        return big_int;
    }
    if(big_int.zero()) {
        return *this;
    }
    
    bint result (0);
    size_t common_size;
    size_t bigger_size;
    decltype(_number_)* biggerp;
    if(_virtual_number_size_ > big_int._virtual_number_size_) {
        common_size = big_int._virtual_number_size_;
        bigger_size = _virtual_number_size_;
        biggerp = const_cast<std::vector<unsigned char>*>(&_number_);
    } else {
        common_size = _virtual_number_size_;
        bigger_size = big_int._virtual_number_size_;
        biggerp = const_cast<std::vector<unsigned char>*>(&big_int._number_);
    }
    
    for(size_t i=0; i < common_size; ++i) {
        const auto a = _number_.at(i);
        const auto b = big_int._number_.at(i);
        const auto r = a | b;
        result.push_back(r);
    }
    
    const auto& bigger = *biggerp;
    for(size_t i = common_size; i < bigger_size; ++i) {
        const auto a = bigger.at(i);
        result.push_back(a);
    }
    
    if(bigger_size != common_size) {
        result._is_zero_ = false;
        result._is_negative_ = _is_negative_ or big_int._is_negative_;
    } else {
        result.update_virtual_number_size();
        if(result.zero() == false) {
            result._is_negative_ = _is_negative_ or big_int._is_negative_;
        }
    }
    
    return result;
}

bint bint::operator^(const bint& big_int) const {
    if(zero() && big_int.zero()) {
        return {0};
    }
    if(zero()) {
        return big_int;
    }
    if(big_int.zero()) {
        return *this;
    }
    
    bint result (0);
    size_t common_size;
    size_t bigger_size;
    decltype(_number_)* biggerp;
    if(_virtual_number_size_ > big_int._virtual_number_size_) {
        common_size = big_int._virtual_number_size_;
        bigger_size = _virtual_number_size_;
        biggerp = const_cast<std::vector<unsigned char>*>(&_number_);
    } else {
        common_size = _virtual_number_size_;
        bigger_size = big_int._virtual_number_size_;
        biggerp = const_cast<std::vector<unsigned char>*>(&big_int._number_);
    }
    
    for(size_t i=0; i < common_size; ++i) {
        const auto a = _number_.at(i);
        const auto b = big_int._number_.at(i);
        const auto r = a ^ b;
        result.push_back(r);
    }
    
    const auto& bigger = *biggerp;
    for(size_t i = common_size; i < bigger_size; ++i) {
        const auto a = bigger.at(i); // a = a ^ 0; - não é necessário fazer a operação
        result.push_back(a);
    }
    
    if(bigger_size != common_size) {
        result._is_zero_ = false;
        result._is_negative_ = _is_negative_ xor big_int._is_negative_;
    } else {
        result.update_virtual_number_size(bigger_size);
        if(result.zero() == false) {
            result._is_negative_ = _is_negative_ xor big_int._is_negative_;
        }
    }
    
    return result;
}

bint bint::operator~() const {
    if(zero()) {
        return {1};
    }
    
    bint result (0);
    for(size_t i=0; i < _virtual_number_size_; ++i) {
        const auto a = _number_.at(i);
        const auto r = ~a;
        result.push_back(r);
    }
    
    result.update_virtual_number_size();
    if(result.zero() == false) {
        result._is_negative_ = !_is_negative_;
    }
    
    return result;
}

bint bint::operator<<(const long long n) const {
    if(n < 0) {
        throw binterror("the number of 'shifting' must be a integer positive number. n: ",n);
        // show("ERROR - the number of 'shifting' must be a integer positive number. n: ",n,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        return {0};
    }
    if(n == 0) {
        return *this;
    }
    /**
     * Encontra quantos blocos totalmente zero serão adicionados: quociente;
     * Encontra qual o valor do primeiro shift interno que será feito de fato dentro dos bits: remainder da divisão por 8.
     * 8 bits = 1 byte = 1 char.
    */
    const auto shift = lldiv(n, _base_bit_size_);
    
    bint result (0);
    
    // insere os blocos completos de zero no começo do número.
    for(size_t i=0; i < static_cast<size_t>(shift.quot); ++i) {
        result.push_back(0);
    }
    
    if(shift.rem == 0) { // o shift é perfeito e não precisa de correção
        for(size_t i=0; i < _virtual_number_size_; ++i) {
            const int a = static_cast<int>(_number_.at(i));
            result.push_back(a);
        }
        
    } else {
        const int overflow_shift = _base_bit_size_ - shift.rem;
        unsigned char overflow = 0;
        unsigned char new_overflow = 0;
        for(size_t i=0; i < _virtual_number_size_; ) {
            unsigned long long word = 0;
        
            int end = -1;
            for(int j=7; j > -1; --j) {
                if(i+j < _virtual_number_size_) {
                    if(end == -1) {
                        end = j;
                        new_overflow = _number_.at(i+j) >> overflow_shift;
                        word = _number_.at(i+j);
                    } else {
                        word = word << _base_bit_size_;
                        word = word | _number_.at(i+j);
                    }
                }
            }
        
            word = word << shift.rem;
            word = word | overflow;
            overflow = new_overflow;
        
            for(int j=0; j <= end; ++j) {
                const int r = word & _base_mask_;
                result.push_back(r);
                word = word >> _base_bit_size_;
            }
        
            i = i + end + 1; // atualiza o valor de i
        }
    
        if(overflow != 0) {
            result.push_back(overflow);
        }
    }
    
    result.update_virtual_number_size();
    if(result.zero() == false) {
        result._is_negative_ = _is_negative_;
    }
    
    return result;
}

bint bint::operator>>(const long long n) const {
    if(n < 0) {
        throw binterror("the number of 'shifting' must be a integer positive number. n: ",n);
        // show("ERROR - the number of 'shifting' must be a integer positive number. n: ",n,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        return {0};
    }
    if(n == 0) {
        return *this;
    }
    
    /**
     * Encontra quantos blocos totalmente zero serão retirados: quociente;
     * Encontra qual o valor do primeiro shift interno que será feito de fato dentro dos bits: remainder da divisão por 8.
     * 8 bits = 1 byte = 1 char.
    */
    const auto shift = lldiv(n, _base_bit_size_);
    bint result (0);
    
    if(shift.rem == 0) { // shift perfeito, somente inseri os bits restantes no resultado
        for(size_t i=shift.quot; i < _virtual_number_size_; ++i) {
            const int a = static_cast<int>(_number_.at(i));
            result.push_back(a);
        }
        
    } else {
        const int overflow_shift = _base_bit_size_ - shift.rem;
        const int overflow_mask = (1 << shift.rem) -1;
        
        for(size_t i = shift.quot; i < _virtual_number_size_-1; ++i) {
            const int a = _number_.at(i+1) & overflow_mask;
            const int b = a << overflow_shift;
            const int c = _number_.at(i) >> shift.rem;
            const int r = b | c;
            
            result.push_back(r);
        }
    
        const int f = _number_.at(_virtual_number_size_-1) >> shift.rem;
        result.push_back(f); // insere o último elemento
    }
    
    result.update_virtual_number_size(_virtual_number_size_-1);
    if(result.zero() == false) {
        result._is_negative_ = _is_negative_;
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .cpp file - self operations
///////////////////////////////////////////////////////////////////////////////////////
bint& bint::mul(const int number) {
    if(zero()) {
        return *this;
    }
    if(number == 0) {
        set_zero();
    }
    if(number == 1) {
        return *this;
    }
    if(number == -1) {
        _is_negative_ = !_is_negative_;
        return *this;
    }
    if(number < 0) {
        _is_negative_ = !_is_negative_;
    }
    
    const unsigned long long num = number < 0 ? -1*number : number;
    unsigned long long overflow = 0;
    size_t i = 0;
    for(; i < _virtual_number_size_; ++i) {
        const auto a = static_cast<unsigned long long>(_number_.at(i));
        const auto r1 = a*num + overflow;
        const auto r2 = r1 & _base_mask_;
        overflow = r1 >> _base_bit_size_;
        _number_.at(i) = static_cast<unsigned char>(r2);
    }
    
    while(overflow > 0) {
        push_back(overflow);
        overflow = overflow >> _base_bit_size_;
    }
    
    return *this;
}

bint& bint::add(const bint& number) {
    if(number.zero()) {
        return *this;
    }
    if(zero()) {
        _is_negative_ = number._is_negative_;
        _is_zero_ = number._is_zero_;
        _virtual_number_size_ = number._virtual_number_size_;
        _number_ = number._number_;
        return *this;
    }
    
    if(_is_negative_ == number._is_negative_) {
        private_add(number);
        
    } else {
        switch(cmp_abs(number)) {
            case  0: set_zero(); break;
            case  1: private_sub_self_minuend(number); break;
            case -1: private_sub_self_subtrahend(number); break;
            default: throw binterror("invalid result from cmp_abs.");
            // default: show("ERROR - invalid result from cmp_abs. line: ",__LINE__,"\n"); exit(EXIT_FAILURE);
        }
    }
    
    return *this;
}

bint& bint::sub(const bint& number) {
    if(number.zero()) {
        return *this;
    }
    if(zero()) {
        _is_negative_ = !number._is_negative_; // tem o sinal contrário
        _is_zero_ = number._is_zero_;
        _virtual_number_size_ = number._virtual_number_size_;
        _number_ = number._number_;
        return *this;
    }
    
    if(_is_negative_ != number._is_negative_) {
        private_add(number);
        
    } else {
        switch(cmp_abs(number)) {
            case  0: set_zero(); break;
            case  1: private_sub_self_minuend(number); break;
            case -1: private_sub_self_subtrahend(number);
                     _is_negative_ = !number._is_negative_; break; // acerta o sinal do resultado
            default: throw binterror("invalid result from cmp_abs.");
            // default: show("ERROR - invalid result from cmp_abs. line: ",__LINE__,"\n"); exit(EXIT_FAILURE);
        }
    }
    
    return *this;
}

bint& bint::shift_l1() {
    if(zero()) {
        return *this;
    }
    
    constexpr const char last_bit_mask = 128; // para base 256 - unsigned char -> 8 bits
    unsigned char bit = (_number_.at(0) & last_bit_mask) == 0 ? 0 : 1;
    _number_.at(0) = _number_.at(0) << 1;
    for(size_t i=1; i < _virtual_number_size_; ++i) {
        const unsigned char last_bit = (_number_.at(i) & last_bit_mask) == 0 ? 0 : 1;
        _number_.at(i) = _number_.at(i) << 1;
        _number_.at(i) = _number_.at(i) | bit;
        bit = last_bit;
    }

    if(bit == 1) {
        push_back(1);
    }

    return *this;
}

bint& bint::shift_left(const long long n) {
    if(n < 0) {
        throw binterror("the number of 'shifting' must be a integer positive number. n: ",n);
        // show("ERROR - the number of 'shifting' must be a integer positive number. n: ",n,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        return *this;
    }
    if(n == 0) {
        return *this;
    }
    /**
     * Encontra quantos blocos totalmente zero serão adicionados: quociente;
     * Encontra qual o valor do primeiro shift interno que será feito de fato dentro dos bits: remainder da divisão por 8.
     * 8 bits = 1 byte = 1 char.
    */
    const auto shift = lldiv(n, _base_bit_size_);

    // check if _virtual_number_size + shift.quot + 1 > size_t::max

    while((_virtual_number_size_ + shift.quot + 1) >= _number_.size()) {
        const size_t new_size = 256+_number_.size(); // atention to check this point
        _number_.resize(new_size);
    }
    
    if(shift.rem == 0) { // o shift é perfeito e não precisa de correção
        for(size_t i=_virtual_number_size_-1; i != -1; --i) {
            const size_t new_id = i + shift.quot;
            _number_.at(new_id) = _number_.at(i);
        }
        
    } else {
        const unsigned char num_bit_shiftl = _base_bit_size_ - shift.rem; // for _base_ = 256 -> 8 bits.
        // realiza a última posição
        // const decltype(typename std::decay<decltype(_number_.at(0))>::type) lbit = _number_.at(_virtual_number_size_-1) >> num_bit_shiftl;
        const unsigned char lbit = _number_.at(_virtual_number_size_-1) >> num_bit_shiftl;
        _number_.at(_virtual_number_size_+shift.quot) = lbit;

        for(size_t i=_virtual_number_size_-1; i > 0; --i) {
            // const decltype(typename std::decay<decltype(_number_.at(0))>::type) bit = _number_.at(i-1) >> num_bit_shiftl;
            const unsigned char bit = _number_.at(i-1) >> num_bit_shiftl;
            const auto a = _number_.at(i) << shift.rem;
            const auto b = a | bit;
            _number_.at(i+shift.quot) = b;
        }

        // realiza a primeira posição
        _number_.at(shift.quot) = _number_.at(0) << shift.rem;
    }

    for(long long i=0; i < shift.quot; ++i) {
        _number_.at(i) = 0;
    }

    update_virtual_number_size();
    return *this;
}

bint& bint::shift_r1() {
    if(zero()) {
        return *this;
    }

    if(*this == 1 || *this == -1) {
        set_zero();
        return *this;
    }

    constexpr const unsigned char last_bit_mask = 128;

    for(size_t i=0; i < _virtual_number_size_-1; ++i) {
        const auto last_bit = (_number_.at(i+1) & 1) == 0 ? 0 : 1;
        const auto a = _number_.at(i) >> 1;
        if(last_bit == 1) {
            _number_.at(i) = a | last_bit_mask;
        } else {
            _number_.at(i) = a;
        }
    }
    _number_.at(_virtual_number_size_-1) = _number_.at(_virtual_number_size_-1) >> 1;
    // update_virtual_number_size();

    if(_number_.at(_virtual_number_size_-1) == 0) {
        _virtual_number_size_ = _virtual_number_size_ -1;
    }

    return *this;
}

bint& bint::shift_right(const long long n) {
    if(n < 0) {
        throw binterror("the number of 'shifting' must be a integer positive number. n: ",n);
        // show("ERROR - the number of 'shifting' must be a integer positive number. n: ",n,", line: ",__LINE__,"\n");
        // exit(EXIT_FAILURE);
    }
    if(zero()) {
        return *this;
    }
    if(n == 0) {
        return *this;
    }
    
    /**
     * Encontra quantos blocos totalmente zero serão retirados: quociente;
     * Encontra qual o valor do primeiro shift interno que será feito de fato dentro dos bits: remainder da divisão por 8.
     * 8 bits = 1 byte = 1 char.
    */
    const auto shift = lldiv(n, _base_bit_size_);

    if(static_cast<size_t>(shift.quot) >= _virtual_number_size_) {
        set_zero();
        return *this;
    }
    
    if(shift.rem == 0) { // shift perfeito, somente inseri os bits restantes no resultado
        for(size_t i=shift.quot; i < _virtual_number_size_; ++i) {
            const auto a = _number_.at(i);
            const size_t new_id = i - shift.quot;
            // _number_.at(i) = 0; // deve ser antes para o caso de i == new_id -> shift.quot = 0
            _number_.at(new_id) = a;
        }
        
    } else {
        const int overflow_shift = _base_bit_size_ - shift.rem;
        const int overflow_mask = (1 << shift.rem) -1;
        
        for(size_t i = shift.quot; i < _virtual_number_size_-1; ++i) {
            const int a = _number_.at(i+1) & overflow_mask;
            const int b = a << overflow_shift;
            const int c = _number_.at(i) >> shift.rem;
            const int r = b | c;
            const size_t new_id = i - shift.quot;
            // _number_.at(i) = 0; // deve ser antes para o caso de i == new_id -> shift.quot = 0
            _number_.at(new_id) = static_cast<unsigned char>(r);
        }
    
        const auto f = _number_.at(_virtual_number_size_-1) >> shift.rem;
        const size_t new_lid = _virtual_number_size_ -1 - shift.quot;
        // _number_.at(_virtual_number_size_-1) = 0; // deve ser antes para o caso de i == new_id -> shift.quot = 0
        _number_.at(new_lid) = static_cast<unsigned char>(f); // insere o último elemento
    }

    // reseta todos as casas do número que agora são 0
    for(size_t i= _virtual_number_size_ - shift.quot; i < _virtual_number_size_; ++i) {
        _number_.at(i) = 0;
    }
    
    update_virtual_number_size(_virtual_number_size_-1);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
// others operations
///////////////////////////////////////////////////////////////////////////////////////
bint bint::gcd(const bint& big_int) const {
    if(zero() == true) {
        return big_int.abs();
    }

    if(big_int.zero() == true) {
        return abs();
    }

    if(cmp_abs(1) == 0 || big_int.cmp_abs(1) == 0) {
        return 1;
    }

    const auto cmp = cmp_abs(big_int);
    if(cmp == 0) {
        return abs();
    }

    bint dividend = 0;
    bint divisor = 0;
    if(cmp == 1) {
        dividend = abs();
        divisor  = big_int.abs();
    } else { // cmp == -1
        dividend = big_int.abs();
        divisor  = abs();
    }

    auto remainder = dividend % divisor;
    while(remainder != 0) {
        dividend = divisor;
        divisor = remainder;
        remainder = dividend % divisor;
    }

    return divisor;
}

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .cpp file - auxiliar functions
///////////////////////////////////////////////////////////////////////////////////////
int bint::cmp(const bint& number) const {
    if(this->operator==(number)) return 0;
    if(this->operator>(number)) {
        if(_is_negative_ == number._is_negative_) {
            return 1;
        } else {
            return 2;
        }
    } else { // this < number
        if(_is_negative_ == number._is_negative_) {
            return -1;
        } else {
            return -2;
        }
    }
}

int bint::cmp_abs(const bint& number) const {
    if(_virtual_number_size_ > number._virtual_number_size_) {
        return 1;
    } else if(_virtual_number_size_ < number._virtual_number_size_) {
        return -1;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // ambos os números tem o mesmo tamanho (número de blocos)
    ////////////////////////////////////////////////////////////////////////////
    if(_virtual_number_size_ == 0) {
        return 0;
    }
    if(_virtual_number_size_ == 1) {
        if(_number_.at(0) > number._number_.at(0)) {
            return 1;
        } else if(_number_.at(0) < number._number_.at(0)) {
            return -1;
        } else {
            return 0;
        }
    }
    
    // for(auto i = Iterator_Size_t(_virtual_number_size_, _virtual_number_size_-1, 0);
    //     i.end() == false; --i) {
    //     if(_number_.at(i()) > number._number_.at(i())) {
    //         return 1;
    //     } else if(_number_.at(i()) < number._number_.at(i())) {
    //         return -1;
    //     }
    // }
    for(size_t i = _virtual_number_size_-1; i != -1; --i) {
        if(_number_.at(i) > number._number_.at(i)) {
            return 1;
        } else if(_number_.at(i) < number._number_.at(i)) {
            return -1;
        }
    }
    
    return 0;
}

bint bint::private_subtration(const bint& minuend, const bint& subtrahend) const {
    bint result (0);
    
    const auto equal_size = subtrahend._virtual_number_size_;
    bool has_overflow = false;
    for(size_t i=0; i < equal_size; ++i) {
        const int m1 = static_cast<int>(minuend._number_.at(i));
        const int m2 = has_overflow == true ? m1-1 : m1;
        const int s = static_cast<int>(subtrahend._number_.at(i));
        int r = 0;
        if(m2 < s) {
            r = m2+256-s;
            has_overflow = true;
        } else {
            r = m2-s;
            has_overflow = false;
        }
        
        result.push_back(r);
    }
    
    for(size_t i=equal_size; i < minuend._virtual_number_size_; ++i) {
        const int m1 = static_cast<int>(minuend._number_.at(i));
        const int m2 = has_overflow == true ? m1-1 : m1;
        int r = 0;
        if(m2 < 0) {
            r = 255;
            has_overflow = true;
        } else {
            r = m2;
            has_overflow = false;
        }
        
        result.push_back(r);
    }
    
    result.update_virtual_number_size();
    if(result.zero() == false) {
        result._is_negative_ = minuend._is_negative_;
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////
// implementation in .cpp file - auxiliar functions - self operations
///////////////////////////////////////////////////////////////////////////////////////
void bint::private_add(const bint& number) {
    int overflow = 0;
    const size_t size = _virtual_number_size_ > number._virtual_number_size_ ? number._virtual_number_size_ : _virtual_number_size_;
    for(size_t i=0; i < size; ++i) {
        const auto a = static_cast<int>(_number_.at(i));
        const auto b = static_cast<int>(number._number_.at(i));
        const auto r1 = a + b + overflow;
        const auto r2 = r1 & _base_mask_;
        _number_.at(i) = static_cast<unsigned char>(r2); // update the result
        
        overflow = r1 >> _base_bit_size_;
    }
    
    if(_virtual_number_size_ > number._virtual_number_size_) {
        for(size_t i = number._virtual_number_size_; i < _virtual_number_size_; ++i) {
            const auto a = static_cast<int>(_number_.at(i));
            const auto r1 = a + overflow;
            const auto r2 = r1 & _base_mask_;
            _number_.at(i) = static_cast<unsigned char>(r2); // update the result
        
            overflow = r1 >> _base_bit_size_;
        }
        
    } else if(_virtual_number_size_ < number._virtual_number_size_) {
        for(size_t i = _virtual_number_size_; i < number._virtual_number_size_; ++i) {
            const auto a = static_cast<int>(number._number_.at(i));
            const auto r1 = a + overflow;
            const auto r2 = r1 & _base_mask_;
            push_back(r2); // update the result
        
            overflow = r1 >> _base_bit_size_;
        }
    }
    
    // only left overflow to count
    while(overflow > 0) {
        const auto r = overflow & _base_mask_;
        push_back(r);
        overflow = overflow >> _base_bit_size_;
    }
}

void bint::private_sub_self_minuend(const bint& subtrahend) {
    bool overflow = false;
    for(size_t i=0; i < subtrahend._virtual_number_size_; ++i) {
        int m = static_cast<int>(_number_.at(i));
        m = overflow == true ? m-1 : m;
        
        const int s = static_cast<int>(subtrahend._number_.at(i));
        if(m < s) {
            m = m + _base_;
            overflow = true;
        } else {
            overflow = false;
        }
        
        const int r = m - s;
        _number_.at(i) = static_cast<unsigned char>(r);
    }
    
    for(size_t i=subtrahend._virtual_number_size_; i < _virtual_number_size_; ++i) {
        if(overflow == true) {
            const int m = static_cast<int>(_number_.at(i)) -1;
            if(m < 0) {
                _number_.at(i) = static_cast<unsigned char>(_base_mask_);
                overflow = true;
            } else {
                _number_.at(i) = static_cast<unsigned char>(m);
                overflow = false;
            }
        } else {
            overflow = false;
        }
    }
    
    update_virtual_number_size(_virtual_number_size_ -1);
}

void bint::private_sub_self_subtrahend(const bint& minuend) {
    bool overflow = false;
    for(size_t i=0; i < _virtual_number_size_; ++i) {
        int m = static_cast<int>(minuend._number_.at(i));
        m = overflow == true ? m-1 : m;
        
        const int s = static_cast<int>(_number_.at(i));
        if(m < s) {
            m = m + _base_;
            overflow = true;
        } else {
            overflow = false;
        }
        
        const int r = m - s;
        _number_.at(i) = static_cast<unsigned char>(r);
    }
    
    for(size_t i=_virtual_number_size_; i < minuend._virtual_number_size_; ++i) {
        if(overflow == true) {
            const int m = static_cast<int>(minuend._number_.at(i)) -1;
            if(m < 0) {
                push_back(_base_mask_);
                overflow = true;
            } else {
                push_back(m);
                overflow = false;
            }
        } else {
            const int m = static_cast<int>(minuend._number_.at(i));
            push_back(m);
            overflow = false;
        }
    }
    
    update_virtual_number_size();
    if(zero() == false) {
        _is_negative_ = minuend._is_negative_;
    }
}

void bint::private_sub_self_minuend_for_div(const bint& subtrahend, const size_t begin) {
    bool overflow = false;
    for(size_t i=0; i < subtrahend._virtual_number_size_; ++i) {
        int m = static_cast<int>(_number_.at(i+begin));
        m = overflow == true ? m-1 : m;
        
        const int s = static_cast<int>(subtrahend._number_.at(i));
        if(m < s) {
            m = m + _base_;
            overflow = true;
        } else {
            overflow = false;
        }
        
        const int r = m - s;
        _number_.at(i+begin) = static_cast<unsigned char>(r);
    }
    
    for(size_t i=subtrahend._virtual_number_size_+begin; i < _virtual_number_size_; ++i) {
        if(overflow == true) {
            const int m = static_cast<int>(_number_.at(i)) -1;
            if(m < 0) {
                _number_.at(i) = static_cast<unsigned char>(_base_mask_);
                overflow = true;
            } else {
                _number_.at(i) = static_cast<unsigned char>(m);
                overflow = false;
            }
        } else {
            overflow = false;
        }
    }
    
    update_virtual_number_size(_virtual_number_size_ -1);
}
