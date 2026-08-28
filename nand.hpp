#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <cstdint>
#include <array>

#define print(x) std::cout << x << std::endl

#define hex(x) std::hex << (x) << std::dec

#ifdef DEBUG
    #define debug(x) print("\033[32m[" << __FILE__ << ':' << __LINE__ << "]\033[0m " << x)

    #define trace() print("\033[33m[" << __FILE__ << ':' << __LINE__ << "]\033[0m " << __PRETTY_FUNCTION__)
#else
    #define debug(x)
    #define trace()
#endif

#define assert(c) do{ \
    if (!(c)){ \
        print("\033[31m[ERROR " << __FILE__ << ':' << __LINE__ << "]\033[0m Assertion failed: " << #c); \
        exit(-1); \
    } \
}while(0)


uint64_t _hrand_state = -1;


uint64_t hrand(){
    uint64_t result = _hrand_state;

    _hrand_state += std::rotl(_hrand_state, 17) + std::rotl(_hrand_state, 29) + std::rotl(_hrand_state, 43);

    return result;
}


void shrand(uint64_t seed = 0){
    _hrand_state = ~seed * 0x9E3779B97F4A7C15;
    _hrand_state ^= std::rotl(_hrand_state, 37);

    hrand();
}



namespace nand{

enum class Value: uint8_t{
    Zero, One, X, Z
};


::std::ostream& operator<<(std::ostream& os, Value obj){
    if (obj == Value::X) os << 'X';
    else if (obj == Value::Z) os << 'Z';
    else os << (obj == Value::One ? '1' : '0');

    return os;
};


class Signal;
class Simulator;


class Component{
    friend class Simulator;
    friend class Signal;

protected:
    Simulator* sim;
    std::vector<Signal*> inputs;
    std::vector<Signal*> outputs;
    bool is_dirty = false;

    void connect(Signal* signal, int in_pin){
        assert(in_pin >= 0);
        assert(in_pin < inputs.size());

        inputs[in_pin] = signal;
    }

    void connect(int out_pin, Signal* signal){
        assert(out_pin >= 0);
        assert(out_pin < outputs.size());

        outputs[out_pin] = signal;
    }

    void dirt();

public:
    Component(){ trace(); }

    virtual ~Component(){ trace(); };

    virtual void eval(){ trace(); }
};


class Signal{
    friend class Simulator;
    friend class Component;

    Value value;
    std::vector<Component*> listeners;

    void connect(Component* component){
        assert(component != nullptr);

        listeners.push_back(component);
    }

public:
    Signal(Value init = Value::X): value(init){ trace(); }

    ~Signal(){ trace(); }

    Value get(){ return value; }

    void set(Value val = Value::X){
        trace();

        if (value != val){
            value = val;

            for (auto l: listeners) l->dirt();
        }
    }

    void random(){
        set((hrand() & 1) ? Value::One : Value::Zero);
    }
};


template<int N>
class Bus: std::array<Signal*, N>{
public:
    Bus(){ trace(); }

    void random(){
        trace();

        for (Signal* signal: this){
            assert(signal != nullptr);

            signal->random();
        }
    }

    void set(Value value, int pin){
        trace();

        assert(pin >= 0);
        assert(pin < N);

        assert(this[pin] != nullptr);

        this[pin]->set(value);
    }

    int width(){ return N; }
};


class Simulator{
    std::vector<Signal*> signals;
    std::vector<Component*> components;
    std::deque<Component*> dirty;

public:
    Simulator(){ trace(); }

    ~Simulator(){
        trace();

        for (auto s: signals) delete s;
        for (auto g: components) delete g;
    }

    void makeDirty(Component* component){
        trace();

        assert(component != nullptr);

        if (!component->is_dirty){
            component->is_dirty = true;
            dirty.push_front(component);
        }
    }

    template<typename Comp>
    Comp* create(std::initializer_list<Signal*> inputs = {}, std::initializer_list<Signal*> outputs = {}){
        trace();
        
        Comp* component = new Comp();

        component->sim = this;

        std::vector<Signal*> inp(inputs), out(outputs);

        for (int i = 0; i < inp.size(); ++i) connect(component, inp[i], i);

        for (int i = 0; i < out.size(); ++i) connect(component, i, out[i]);

        makeDirty(component);

        return component;
    }

    template<int N>
    Bus<N>* create(std::initializer_list<Signal*> init = {}){
        trace();

        Bus<N>* bus = new Bus<N>();

        std::vector<Signal*> inputs(init);

        assert(inputs.size() <= N);

        for (int i = 0; i < N; ++i){
            if (i < inputs.size()) (*bus)[i] = inputs[i];
            else (*bus)[i] = create();
        }

        return bus;
    }

    Signal* create(Value value = Value::X){
        trace();

        Signal* s = new Signal(value);

        signals.push_back(s);

        return s;
    }

    void connect(Component* component, Signal* signal, int in_pin){
        trace();

        assert(component != nullptr);
        assert(signal != nullptr);

        component->connect(signal, in_pin);
        signal->connect(component);
    }

    void connect(Component* component, int out_pin, Signal* signal){
        trace();

        assert(component != nullptr);
        assert(signal != nullptr);

        component->connect(out_pin, signal);
    }

    void eval(){
        trace();

        while (!dirty.empty()){
            Component* top = dirty.front();
            dirty.pop_front();

            if (top->is_dirty){
                top->is_dirty = false;
                top->eval();
            }
        }
    }
};


void Component::dirt(){
    sim->makeDirty(this);
}


class NandGate: public Component{
public:
    NandGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();
        Value b = inputs[1]->get();
        
        if (a == Value::Zero || b == Value::Zero) outputs[0]->set(Value::One);
        else if (a == Value::One && b == Value::One) outputs[0]->set(Value::Zero);
        else outputs[0]->set(Value::X);
    }
};


class NorGate: public Component{
public:
    NorGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();
        Value b = inputs[1]->get();

        if (a == Value::One || b == Value::One) outputs[0]->set(Value::Zero);
        else if (a == Value::Zero && b == Value::Zero) outputs[0]->set(Value::One);
        else outputs[0]->set(Value::X);
    }
};


class InvGate: public Component{
public:
    InvGate(): Component(){
        inputs.resize(1);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();

        if (a == Value::Zero) outputs[0]->set(Value::One);
        else if (a == Value::One) outputs[0]->set(Value::Zero);
        else outputs[0]->set(Value::X);
    }
};


class AndGate: public Component{
public:
    AndGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();
        Value b = inputs[1]->get();

        if (a == Value::Zero || b == Value::Zero) outputs[0]->set(Value::Zero);
        else if (a == Value::One && b == Value::One) outputs[0]->set(Value::One);
        else outputs[0]->set(Value::X);
    }
};


class OrGate: public Component{
public:
    OrGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();
        Value b = inputs[1]->get();

        if (a == Value::One || b == Value::One) outputs[0]->set(Value::One);
        else if (a == Value::Zero && b == Value::Zero) outputs[0]->set(Value::Zero);
        else outputs[0]->set(Value::X);
    }
};


class XorGate: public Component{
public:
    XorGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();
        Value b = inputs[1]->get();

        if (a == Value::X || a == Value::Z || b == Value::X || b == Value::Z) outputs[0]->set(Value::X);
        else if (a != b) outputs[0]->set(Value::One);
        else outputs[0]->set(Value::Zero);
    }
};


class XnorGate: public Component{
public:
    XnorGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value a = inputs[0]->get();
        Value b = inputs[1]->get();

        if (a == Value::X || a == Value::Z || b == Value::X || b == Value::Z) outputs[0]->set(Value::X);
        else if (a == b) outputs[0]->set(Value::One);
        else outputs[0]->set(Value::Zero);
    }
};


class MuxGate: public Component{
public:
    MuxGate(): Component(){
        inputs.resize(3);
        outputs.resize(1);
    }

    void eval(){
        Value sel = inputs[0]->get();
        Value a = inputs[1]->get();
        Value b = inputs[2]->get();

        if (sel == Value::Z || sel == Value::X) outputs[0]->set(Value::X);

        else if (sel == Value::Zero){
            if (a == Value::Z) outputs[0]->set(Value::X);
            else outputs[0]->set(a);
        }else{
            if (b == Value::Z) outputs[0]->set(Value::X);
            else outputs[0]->set(b);
        }
    }
};


class DemuxGate: public Component{
public:
    DemuxGate(): Component(){
        inputs.resize(2);
        outputs.resize(2);
    }

    void eval(){
        Value s = inputs[0]->get();
        Value d = inputs[1]->get();

        if (d == Value::Z) d = Value::X;

        if (s == Value::Zero){
            outputs[0]->set(d);
            outputs[1]->set(Value::Zero);
        }
        else if (s == Value::One){
            outputs[0]->set(Value::Zero);
            outputs[1]->set(d);
        }
        else{
            outputs[0]->set(Value::X);
            outputs[1]->set(Value::X);
        }
    }
};


class DLatchGate: public Component{
    Value state = Value::X;

public:
    DLatchGate(): Component(){
        inputs.resize(2);
        outputs.resize(1);
    }

    void eval(){
        Value e = inputs[0]->get();
        Value d = inputs[1]->get();

        if (d == Value::Z) d = Value::X;

        if (e == Value::One) state = d;
        else if (e == Value::X || e == Value::Z){
            if (state != d) state = Value::X;
        }

        outputs[0]->set(state);
    }
};


class DFFGate: public Component{
    Value state = Value::X;
    Value prev_clk = Value::X;

public:
    DFFGate(): Component(){
        inputs.resize(2);
        outputs.resize(2);
    }

    void eval(){
        Value d = inputs[0]->get();
        Value clk = inputs[1]->get();

        if (clk == Value::Z) clk = Value::X;
        if (d == Value::Z) d = Value::X;

        if (prev_clk == Value::Zero && clk == Value::One) state = d;

        else if ((prev_clk == Value::Zero && clk == Value::X) || (prev_clk == Value::X && clk == Value::One)){
            if (state != d) state = Value::X;
        }

        prev_clk = clk;
        outputs[0]->set(state);
    }
};


} // nand