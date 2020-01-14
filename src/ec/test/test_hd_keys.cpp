#include "../../common/hex.hpp"
#include "../../common/term.hpp"
#include "../../common/sha1.hpp"
#include "../keys.hpp"

using namespace prologcoin::common;
using namespace prologcoin::ec;

static void header( const std::string &str )
{
    std::cout << "\n";
    std::cout << "--- [" + str + "] " + std::string(60 - str.length(), '-') << "\n";
    std::cout << "\n";
}

// Test vectors taken from BIP32

static void test_vector_1()
{
    header("test_vector_1");

    const uint8_t SEED[16] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
  		  	      0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
  
    secp256k1_ctx ctx;
    hd_keys hd(ctx, SEED, sizeof(SEED));
    std::string master_private, master_public;

    // Chain m

    std::cout << "MASTER PRIVATE KEY: " << (master_private = hd.master_private().to_string()) << std::endl;
    assert(master_private == "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi");
    
    std::cout << "MASTER PUBLIC KEY: " << (master_public = hd.master_public().to_string()) << std::endl;
    assert(master_public == "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8");

    // Chain m/0H

    extended_private_key prv_m_0H;
    extended_public_key pub_m_0H;
    assert(hd.generate_child(hd.master_private(), hd_keys::H(0), prv_m_0H));
    
    std::cout << "Chain m/0H: ext prv: " << prv_m_0H.to_string() << std::endl;
    assert(prv_m_0H.to_string() == "xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rGL5hj6KCesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7");

    assert(hd.generate_child(hd.master_private(), hd_keys::H(0), pub_m_0H));

    std::cout << "Chain m/0H: ext pub: " << pub_m_0H.to_string() << std::endl;
    assert(pub_m_0H.to_string() == "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw");

    // Chain m/0H/1
    extended_private_key prv_m_0H_1;
    extended_public_key pub_m_0H_1;
    assert(hd.generate_child(pub_m_0H, 1, pub_m_0H_1));
    assert(hd.generate_child(prv_m_0H, 1, prv_m_0H_1));
    std::cout << "Chain m/0H/1: ext prv: " << prv_m_0H_1.to_string() << std::endl;
    std::cout << "Chain m/0H/1: ext pub: " << pub_m_0H_1.to_string() << std::endl;
    assert(prv_m_0H_1.to_string() == "xprv9wTYmMFdV23N2TdNG573QoEsfRrWKQgWeibmLntzniatZvR9BmLnvSxqu53Kw1UmYPxLgboyZQaXwTCg8MSY3H2EU4pWcQDnRnrVA1xe8fs");
    assert(pub_m_0H_1.to_string() == "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAWbWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ");

    // Chain m/0H/1/2H
    extended_private_key prv_m_0H_1_2H;
    extended_public_key pub_m_0H_1_2H;
    assert(hd.generate_child(prv_m_0H_1, hd_keys::H(2), pub_m_0H_1_2H));
    assert(hd.generate_child(prv_m_0H_1, hd_keys::H(2), prv_m_0H_1_2H));
    std::cout << "Chain m/0H/1/2H: ext prv: " << prv_m_0H_1_2H.to_string() << std::endl;
    std::cout << "Chain m/0H/1/2H: ext pub: " << pub_m_0H_1_2H.to_string() << std::endl;
    assert(prv_m_0H_1_2H.to_string() == "xprv9z4pot5VBttmtdRTWfWQmoH1taj2axGVzFqSb8C9xaxKymcFzXBDptWmT7FwuEzG3ryjH4ktypQSAewRiNMjANTtpgP4mLTj34bhnZX7UiM");
    assert(pub_m_0H_1_2H.to_string() == "xpub6D4BDPcP2GT577Vvch3R8wDkScZWzQzMMUm3PWbmWvVJrZwQY4VUNgqFJPMM3No2dFDFGTsxxpG5uJh7n7epu4trkrX7x7DogT5Uv6fcLW5");

    // Chain m/0H/1/2H/2
    extended_private_key prv_m_0H_1_2H_2;
    extended_public_key pub_m_0H_1_2H_2;
    assert(hd.generate_child(prv_m_0H_1_2H, 2, pub_m_0H_1_2H_2));
    assert(hd.generate_child(prv_m_0H_1_2H, 2, prv_m_0H_1_2H_2));
    std::cout << "Chain m/0H/1/2H/2: ext prv: " << prv_m_0H_1_2H_2.to_string() << std::endl;
    std::cout << "Chain m/0H/1/2H/2: ext pub: " << pub_m_0H_1_2H_2.to_string() << std::endl;
    assert(prv_m_0H_1_2H_2.to_string() == "xprvA2JDeKCSNNZky6uBCviVfJSKyQ1mDYahRjijr5idH2WwLsEd4Hsb2Tyh8RfQMuPh7f7RtyzTtdrbdqqsunu5Mm3wDvUAKRHSC34sJ7in334");
    assert(pub_m_0H_1_2H_2.to_string() == "xpub6FHa3pjLCk84BayeJxFW2SP4XRrFd1JYnxeLeU8EqN3vDfZmbqBqaGJAyiLjTAwm6ZLRQUMv1ZACTj37sR62cfN7fe5JnJ7dh8zL4fiyLHV");

    // Chain m/0H/1/2H/2/1000000000
    extended_private_key prv_m_0H_1_2H_2_1000000000;
    extended_public_key pub_m_0H_1_2H_2_1000000000;
    assert(hd.generate_child(prv_m_0H_1_2H_2, 1000000000, pub_m_0H_1_2H_2_1000000000));
    assert(hd.generate_child(prv_m_0H_1_2H_2, 1000000000, prv_m_0H_1_2H_2_1000000000));
    std::cout << "Chain m/0H/1/2H/2/1000000000: ext prv: " << prv_m_0H_1_2H_2_1000000000.to_string() << std::endl;
    std::cout << "Chain m/0H/1/2H/2/1000000000: ext pub: " << pub_m_0H_1_2H_2_1000000000.to_string() << std::endl;
    assert(prv_m_0H_1_2H_2_1000000000.to_string() == "xprvA41z7zogVVwxVSgdKUHDy1SKmdb533PjDz7J6N6mV6uS3ze1ai8FHa8kmHScGpWmj4WggLyQjgPie1rFSruoUihUZREPSL39UNdE3BBDu76");
    assert(pub_m_0H_1_2H_2_1000000000.to_string() == "xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFgJS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy");
}

static void test_vector_2()
{
    header("test_vector_2");

    uint8_t SEED[64];
    hex::from_string("fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542", SEED, 64);

    secp256k1_ctx ctx;
    hd_keys hd(ctx, SEED, sizeof(SEED));
    std::string master_private, master_public;

    // Chain m

    std::cout << "MASTER PUBLIC KEY: " << (master_public = hd.master_public().to_string()) << std::endl;
    assert(master_public == "xpub661MyMwAqRbcFW31YEwpkMuc5THy2PSt5bDMsktWQcFF8syAmRUapSCGu8ED9W6oDMSgv6Zz8idoc4a6mr8BDzTJY47LJhkJ8UB7WEGuduB");

    std::cout << "MASTER PRIVATE KEY: " << (master_private = hd.master_private().to_string()) << std::endl;
    assert(master_private == "xprv9s21ZrQH143K31xYSDQpPDxsXRTUcvj2iNHm5NUtrGiGG5e2DtALGdso3pGz6ssrdK4PFmM8NSpSBHNqPqm55Qn3LqFtT2emdEXVYsCzC2U");

    // Chain m/0
    extended_public_key pub_m_0;
    extended_private_key prv_m_0;
    assert(hd.generate_child(hd.master_public(), 0, pub_m_0));
    assert(hd.generate_child(hd.master_private(), 0, prv_m_0));

    std::cout << "Chain m/0: ext pub: " << pub_m_0.to_string() << std::endl;
    assert(pub_m_0.to_string() == "xpub69H7F5d8KSRgmmdJg2KhpAK8SR3DjMwAdkxj3ZuxV27CprR9LgpeyGmXUbC6wb7ERfvrnKZjXoUmmDznezpbZb7ap6r1D3tgFxHmwMkQTPH");
    
    std::cout << "Chain m/0: ext prv: " << prv_m_0.to_string() << std::endl;
    assert(prv_m_0.to_string() == "xprv9vHkqa6EV4sPZHYqZznhT2NPtPCjKuDKGY38FBWLvgaDx45zo9WQRUT3dKYnjwih2yJD9mkrocEZXo1ex8G81dwSM1fwqWpWkeS3v86pgKt");

    // Chain m/0/2147483647H
    extended_public_key  pub_m_0_2147483647H;
    extended_private_key prv_m_0_2147483647H;
    assert(hd.generate_child(prv_m_0, hd_keys::H(2147483647), pub_m_0_2147483647H));
    assert(hd.generate_child(prv_m_0, hd_keys::H(2147483647), prv_m_0_2147483647H));

    std::cout << "Chain m/0/2147483647H: ext pub: " << pub_m_0_2147483647H.to_string() << std::endl;
    assert(pub_m_0_2147483647H.to_string() == "xpub6ASAVgeehLbnwdqV6UKMHVzgqAG8Gr6riv3Fxxpj8ksbH9ebxaEyBLZ85ySDhKiLDBrQSARLq1uNRts8RuJiHjaDMBU4Zn9h8LZNnBC5y4a");
    
    std::cout << "Chain m/0/2147483647H: ext prv: " << prv_m_0_2147483647H.to_string() << std::endl;
    assert(prv_m_0_2147483647H.to_string() == "xprv9wSp6B7kry3Vj9m1zSnLvN3xH8RdsPP1Mh7fAaR7aRLcQMKTR2vidYEeEg2mUCTAwCd6vnxVrcjfy2kRgVsFawNzmjuHc2YmYRmagcEPdU9");

    // Chain m/0/2147483647H/1
    extended_public_key  pub_m_0_2147483647H_1;
    extended_private_key prv_m_0_2147483647H_1;
    assert(hd.generate_child(pub_m_0_2147483647H, 1, pub_m_0_2147483647H_1));
    assert(hd.generate_child(prv_m_0_2147483647H, 1, prv_m_0_2147483647H_1));

    std::cout << "Chain m/0/2147483647H/1: ext pub: " << pub_m_0_2147483647H_1.to_string() << std::endl;
    assert(pub_m_0_2147483647H_1.to_string() == "xpub6DF8uhdarytz3FWdA8TvFSvvAh8dP3283MY7p2V4SeE2wyWmG5mg5EwVvmdMVCQcoNJxGoWaU9DCWh89LojfZ537wTfunKau47EL2dhHKon");
    
    std::cout << "Chain m/0/2147483647H/1: ext prv: " << prv_m_0_2147483647H_1.to_string() << std::endl;
    assert(prv_m_0_2147483647H_1.to_string() == "xprv9zFnWC6h2cLgpmSA46vutJzBcfJ8yaJGg8cX1e5StJh45BBciYTRXSd25UEPVuesF9yog62tGAQtHjXajPPdbRCHuWS6T8XA2ECKADdw4Ef");

    // Chain m/0/2147483647H/1/2147483646H
    extended_public_key  pub_m_0_2147483647H_1_2147483646H;
    extended_private_key prv_m_0_2147483647H_1_2147483646H;
    assert(hd.generate_child(prv_m_0_2147483647H_1, hd_keys::H(2147483646), pub_m_0_2147483647H_1_2147483646H));
    assert(hd.generate_child(prv_m_0_2147483647H_1, hd_keys::H(2147483646), prv_m_0_2147483647H_1_2147483646H));

    std::cout << "Chain m/0/2147483647H/1/2147483646H: ext pub: " << pub_m_0_2147483647H_1_2147483646H.to_string() << std::endl;
    assert(pub_m_0_2147483647H_1_2147483646H.to_string() == "xpub6ERApfZwUNrhLCkDtcHTcxd75RbzS1ed54G1LkBUHQVHQKqhMkhgbmJbZRkrgZw4koxb5JaHWkY4ALHY2grBGRjaDMzQLcgJvLJuZZvRcEL");
    
    std::cout << "Chain m/0/2147483647H/1/2147483646H: ext prv: " << prv_m_0_2147483647H_1_2147483646H.to_string() << std::endl;
    assert(prv_m_0_2147483647H_1_2147483646H.to_string() == "xprvA1RpRA33e1JQ7ifknakTFpgNXPmW2YvmhqLQYMmrj4xJXXWYpDPS3xz7iAxn8L39njGVyuoseXzU6rcxFLJ8HFsTjSyQbLYnMpCqE2VbFWc");

    // Chain m/0/2147483647H/1/2147483646H/2
    extended_public_key  pub_m_0_2147483647H_1_2147483646H_2;
    extended_private_key prv_m_0_2147483647H_1_2147483646H_2;
    assert(hd.generate_child(pub_m_0_2147483647H_1_2147483646H, 2, pub_m_0_2147483647H_1_2147483646H_2));
    assert(hd.generate_child(prv_m_0_2147483647H_1_2147483646H, 2, prv_m_0_2147483647H_1_2147483646H_2));

    std::cout << "Chain m/0/2147483647H/1/2147483646H/2: ext pub: " << pub_m_0_2147483647H_1_2147483646H_2.to_string() << std::endl;
    assert(pub_m_0_2147483647H_1_2147483646H_2.to_string() == "xpub6FnCn6nSzZAw5Tw7cgR9bi15UV96gLZhjDstkXXxvCLsUXBGXPdSnLFbdpq8p9HmGsApME5hQTZ3emM2rnY5agb9rXpVGyy3bdW6EEgAtqt");
    
    std::cout << "Chain m/0/2147483647H/1/2147483646H/2: ext prv: " << prv_m_0_2147483647H_1_2147483646H_2.to_string() << std::endl;
    assert(prv_m_0_2147483647H_1_2147483646H_2.to_string() == "xprvA2nrNbFZABcdryreWet9Ea4LvTJcGsqrMzxHx98MMrotbir7yrKCEXw7nadnHM8Dq38EGfSh6dqA9QWTyefMLEcBYJUuekgW4BYPJcr9E7j");

}

static void test_vector_3()
{
    header("test_vector_3");

    uint8_t SEED[64];
    hex::from_string("4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be", SEED, 64);

    secp256k1_ctx ctx;
    hd_keys hd(ctx, SEED, sizeof(SEED));
    std::string master_private, master_public;

    // Chain m

    std::cout << "MASTER PUBLIC KEY: " << (master_public = hd.master_public().to_string()) << std::endl;
    assert(master_public == "xpub661MyMwAqRbcEZVB4dScxMAdx6d4nFc9nvyvH3v4gJL378CSRZiYmhRoP7mBy6gSPSCYk6SzXPTf3ND1cZAceL7SfJ1Z3GC8vBgp2epUt13");

    std::cout << "MASTER PRIVATE KEY: " << (master_private = hd.master_private().to_string()) << std::endl;
    assert(master_private == "xprv9s21ZrQH143K25QhxbucbDDuQ4naNntJRi4KUfWT7xo4EKsHt2QJDu7KXp1A3u7Bi1j8ph3EGsZ9Xvz9dGuVrtHHs7pXeTzjuxBrCmmhgC6");

    // Chaim m/0H

    extended_public_key pub_m_0H;
    extended_private_key prv_m_0H;
    assert(hd.generate_child(hd.master_private(), hd_keys::H(0), pub_m_0H));
    assert(hd.generate_child(hd.master_private(), hd_keys::H(0), prv_m_0H));

    std::cout << "Chain m/0H: ext pub: " << pub_m_0H.to_string() << std::endl;
    assert(pub_m_0H.to_string() == "xpub68NZiKmJWnxxS6aaHmn81bvJeTESw724CRDs6HbuccFQN9Ku14VQrADWgqbhhTHBaohPX4CjNLf9fq9MYo6oDaPPLPxSb7gwQN3ih19Zm4Y");
    
    std::cout << "Chain m/0H: ext prv: " << prv_m_0H.to_string() << std::endl;
    assert(prv_m_0H.to_string() == "xprv9uPDJpEQgRQfDcW7BkF7eTya6RPxXeJCqCJGHuCJ4GiRVLzkTXBAJMu2qaMWPrS7AANYqdq6vcBcBUdJCVVFceUvJFjaPdGZ2y9WACViL4L");
}

int main(int argc, char *argv[])
{
    test_vector_1();
    test_vector_2();
    test_vector_3();

    return 0;
}
