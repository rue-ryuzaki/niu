#include <argparse/argparse_decl.hpp>

#include "image.h"
#include "utils.h"

int
main(int argc,
        char const* const argv[])
{
    auto parent = argparse::ArgumentParser()
            .add_help(false);
    parent.add_argument("-i", "--input")
            .metavar("FILE")
            .required(true)
            .type<std::string>()
            .help("input image file");

    auto& mutex_out = parent.add_mutually_exclusive_group()
            .required(true);
    mutex_out.add_argument("-o", "--output")
            .metavar("FILE")
            .default_value("output.png")
            .type<std::string>()
            .help("output image file");
    mutex_out.add_argument("--overwrite")
            .action("store_true")
            .help("overwrite input file");

    auto parser = argparse::ArgumentParser(argc, argv)
            .formatter_class(argparse::ArgumentDefaultsHelpFormatter)
            .epilog("by rue-ryuzaki (c) 2023");

    auto& subparser = parser.add_subparsers()
            .dest("cmd").required(true);
    subparser.add_parser("create")
            .help("create image")
            .add_argument(argparse::Argument("name").help("image name"))
            .add_argument(argparse::Argument("--size").nargs(2).metavar("W", "H").help("image size"));
    subparser.add_parser("fill")
            .parents(parent)
            .help("fill image")
            .add_argument(argparse::Argument("color").metavar("RRGGBBAA").help("color value"));

    if (argc == 1) {
        parser.print_help();
        return 0;
    }

    auto const args = parser.parse_args();

    auto const command = args.get<std::string>("cmd");

    if (command == "create") {
        auto const output = args.get<std::string>("name");
        auto const vec = args.get<std::vector<uint32_t> >("size");

        auto image = niu::Image::make_image(vec.at(0), vec.at(1));

        if (!image.save(output)) {
            std::cout << "[FAIL] Can't create file '" << output << "'" << std::endl;
            return 1;
        }
        std::cout << "[ OK ] File '" << output << "' generated" << std::endl;
        return 0;
    }

    auto const input = args.get<std::string>("input");
    auto output = args.get<std::string>("o");
    if (args.get<bool>("overwrite")) {
        output = input;
    }

    if (!utils::_is_file_exists(input)) {
        std::cerr << "[FAIL] Input file '" + input + "' not found" << std::endl;
        return 1;
    }

    niu::Image image;
    if (!image.load(input)) {
        std::cerr << "[FAIL] Can't load file '" + input + "' as image" << std::endl;
        return 2;
    }

    if (command == "fill") {
        auto const color = args.get<niu::Color>("color");
        image.fill(color);
    }

    if (!image.save(output)) {
        std::cout << "[FAIL] Can't save file '" << output << "'" << std::endl;
        return 1;
    }
    std::cout << "[ OK ] File '" << output << "' saved" << std::endl;
    return 0;
}
