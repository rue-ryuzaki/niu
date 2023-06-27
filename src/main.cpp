#include <argparse/argparse_decl.hpp>

#include "image.h"

int main(int argc, char const* const argv[])
{
    auto parser = argparse::ArgumentParser(argc, argv);

    parser.parse_args();

    return 0;
}
