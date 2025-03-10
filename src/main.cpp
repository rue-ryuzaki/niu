#include <argparse/argparse_decl.hpp>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "image.h"
#include "utils.h"

int
main(int argc,
        char const* const argv[],
        char const* const envp[])
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

    auto parser = argparse::ArgumentParser(argc, argv, envp)
            .description("niu - niu image utility")
            .allow_abbrev(false)
            .suggest_on_error(true)
            .formatter_class(argparse::ArgumentDefaultsHelpFormatter)
            .fromfile_prefix_chars("@")
            .comment_prefix_chars("#")
            .epilog("by rue-ryuzaki (c) 2023-2024");

    auto& subparser = parser.add_subparsers()
            .dest("cmd").required(true);
    subparser.add_parser("create")
            .help("create image")
            .add_argument(argparse::Argument("name").help("image name"))
            .add_argument(argparse::Argument("--size").nargs(1).metavar("'W H'")
                            .required(true).help("image size"));
    subparser.add_parser("pattern")
            .help("create image from pattern")
            .add_argument(argparse::Argument("name").help("image name"))
            .add_argument(argparse::Argument("-m", "--map").action("append").required(true)
                            .one_or_more().metavar("'S=RRGGBBAA'").help("symbol to color map"))
            .add_argument(argparse::Argument("-r", "--row").action("append").required(true)
                            .one_or_more().help("image row"));
    subparser.add_parser("upscale")
            .parents(parent)
            .help("upscale image")
            .add_argument(argparse::Argument("n").help("upscale multiplier"));
    subparser.add_parser("fill")
            .parents(parent)
            .help("fill image")
            .add_argument(argparse::Argument("color").metavar("RRGGBBAA").help("color value in hex"));
    subparser.add_parser("set_color")
            .parents(parent)
            .help("set color at positions in image")
            .add_argument(argparse::Argument("color").metavar("RRGGBBAA").help("color value in hex"))
            .add_argument(argparse::Argument("-p", "--positions").action("append").required(true)
                            .one_or_more().metavar("'X Y'").help("position"));
    subparser.add_parser("dump")
            .parents(parent)
            .help("dump image");

    if (argc == 1) {
        parser.print_help();
        return 0;
    }

    auto const args = parser.parse_args();

    auto const command = args.get<std::string>("cmd");

    if (command == "create") {
        auto const output = args.get<std::string>("name");
        auto const size = args.get<niu::Vector2>("size");

        auto image = niu::Image::make_image(size.w, size.h);

        if (!image.save(output)) {
            std::cout << "[FAIL] Can't create file '" << output << "'" << std::endl;
            return 1;
        }
        std::cout << "[ OK ] File '" << output << "' generated" << std::endl;
        return 0;
    }

    if (command == "pattern") {
        auto const output = args.get<std::string>("name");
        auto const colormap = args.get<std::map<char, niu::Color> >("map");
        auto const rows = args.get<std::vector<std::string> >("row");

        niu::Vector2 size;
        size.h = rows.size();
        size.w = 0;
        for (auto const& row : rows) {
            if (size.w < row.size()) {
                size.w = row.size();
            }
        }

        auto image = niu::Image::make_image(size.w, size.h);
        image.fill(niu::Color{ });

        for (std::size_t y = 0; y < rows.size(); ++y) {
            for (std::size_t x = 0; x < rows.at(y).size(); ++x) {
                auto symbol = rows.at(y).at(x);
                if (symbol != ' ') {
                    auto color = colormap.at(symbol);
                    image.set_color(x, y, color);
                }
            }
        }

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

    if (!niu::utils::_is_file_exists(input)) {
        std::cerr << "[FAIL] Input file '" + input + "' not found" << std::endl;
        return 1;
    }

    niu::Image image;
    if (!image.load(input)) {
        std::cerr << "[FAIL] Can't load file '" + input + "' as image" << std::endl;
        return 2;
    }

    if (command == "upscale") {
        auto const n = args.get<std::size_t>("n");
        image.upscale(n);
    }

    if (command == "fill") {
        auto const color = args.get<niu::Color>("color");
        image.fill(color);
    }

    if (command == "set_color") {
        auto const color = args.get<niu::Color>("color");
        auto const positions = args.get<std::vector<niu::Vector2> >("positions");
        for (auto const& pos : positions) {
            image.set_color(pos.x, pos.y, color);
        }
    }

    if (command == "dump") {
        if (!image.dump(output)) {
            std::cout << "[FAIL] Can't dump to file '" << output << "'" << std::endl;
            return 1;
        }
    } else if (!image.save(output)) {
        std::cout << "[FAIL] Can't save file '" << output << "'" << std::endl;
        return 1;
    }
    std::cout << "[ OK ] File '" << output << "' saved" << std::endl;
    return 0;
}
