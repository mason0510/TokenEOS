#include "pledge.hpp"

//partII pledge
void pledge::transfer(const name &from, const name &to, const asset &quantity, const string &memo) {
    if (from == get_self() || to != get_self()) {
        return;
    }
    eosio_assert(quantity.is_valid(), "Invalid token transfer...");
   // eosio_assert(quantity.symbol == TOP_SYMBOL, "only TOP token is allowed");
    eosio_assert(quantity.amount > 0, "must be a positive amount");
    changebw(from, quantity, "delegatebw");
}


void pledge::undelegatebw(const name &from,
                          const asset &quantity) {
    eosio_assert(quantity.amount > 0, "must unstake a positive amount");
    changebw(from, quantity, "undelegatebw");
}

#define EOSIO_ABI_EX(TYPE, MEMBERS)	
extern "C"
{
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    print("recver:", name{receiver}, "name", name{code}, "action", name{action});
    print("===========", code, "========", action);
    if (code == receiver) {
        switch (action) {
           // EOSIO_DISPATCH_HELPER(pledge, (undelegatebw))
        EOSIO_API(TYPE, MEMBERS);
        }
    } else if (code == "eosio.token"_n.value && action == "transfer"_n.value) {
        execute_action(name(receiver), name(code), &pledge::transfer);
    }
}
}
EOSIO_ABI_EX(pledge, (undelegatebw))