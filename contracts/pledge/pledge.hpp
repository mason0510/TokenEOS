#include <eosiolib/asset.hpp>

#include <eosiolib/crypto.h>

#include <eosiolib/singleton.hpp> // get or default

#include <eosiolib/time.hpp>

#include <eosiolib/transaction.h>

#define EOS_SYMBOL symbol(symbol_code("EOS"), 4)
#define TOP_SYMBOL symbol(symbol_code("TOP"), 4)

#define TOPTOKEN name("topdapptoken")

using namespace eosio;
using namespace std;
CONTRACT pledge: public eosio::contract {
//partI data
    public: using contract::contract;
    pledge(name receiver,name code, datastream<const char *> ds):contract(receiver, code, ds),_pledgeitem(receiver, receiver.value) {};
    
    ACTION undelegatebw(const name &from,
                    const asset &quantity);


    void transfer(const name &from,
                    const name &to,
                    const asset &quantity,
                    const string &memo);

void changebw(const name &from,
              const asset &quantity, string source) {
    //source

    require_auth(from);
    eosio_assert(quantity.amount != 0, "should be non-zero amount");
    uint64_t key;
    bool exist = false;
    checkExistMem(from, key, exist);
    auto itr = _pledgeitem.find(key);

    if (source == "delegatebw") {
        //save
        if (exist) {
            _pledgeitem.modify(itr, get_self(), [&](auto &p) {
                p.pledgeAmount += quantity.amount;
            });
        } else {
            _pledgeitem.emplace(get_self(), [&](auto &p){
                p.key = _pledgeitem.available_primary_key();
                p.pledgor = from;
                p.pledgeAmount = quantity.amount;
            });
        }
    } else if (source == "undelegatebw") {
        eosio_assert(exist, "not find record ,can not cancel pledge");
        eosio_assert(itr->pledgeAmount >= quantity.amount, "not enough pledge");

        //modify
        _pledgeitem.modify(itr, get_self(), [&](auto &p) {
            p.pledgeAmount -= quantity.amount;
        });
        asset pledgeresult(quantity.amount, EOS_SYMBOL);

        //transfer
        action(
                permission_level{
                        get_self(), "active"_n},
                            "eosio.token"_n, 
                            "transfer"_n,
                std::make_tuple(get_self(), from, pledgeresult, std::string("unpledge from top ")))
        .send();
    }

}
    void checkExistMem(const name &from, uint64_t &key, bool &exist) {
        for (auto &item: _pledgeitem) {
            if (item.pledgor == from) {
                key = item.key;
                exist = true;
                break;
            }
        }
    }

    private:
    TABLE members{
            uint64_t key;
            name pledgor;
            uint64_t pledgeAmount;
            uint64_t primary_key() const {
                return key;
            };
            EOSLIB_SERIALIZE(members, (key)(pledgor)(pledgeAmount));
    };

    typedef eosio::multi_index<name("members"), members> member_index;
    member_index _pledgeitem;
};