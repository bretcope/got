#include <cassert>
#include "FileIR.h"
#include "../io/printing/FmtPosition.h"
#include "../text/Constants.h"

namespace mot
{
    PrefixIR::PrefixIR(const PropertyNode& node, MotString name):
            Node{node},
            Name{std::move(name)}
    {
    }

    PrefixIR::PrefixIR(mot::PrefixIR&& other) noexcept:
            Node{other.Node},
            Name{other.Name},
            PathByEnvironment{std::move(other.PathByEnvironment)},
            OverrideMode{other.OverrideMode}

    {
    }

    FileIR::FileIR(const Console& console, const FileNode& fileNode, bool expectResource):
            _console{console},
            _fileNode{fileNode}
    {
        _isValid = TryBuildIR(fileNode, expectResource);
    }

    bool FileIR::IsValid() const
    {
        return _isValid;
    }

    const ByString<UP<PrefixIR>>& FileIR::Prefixes() const
    {
        return _prefixes;
    }

    bool FileIR::TryBuildIR(const FileNode& fileNode, bool expectResource)
    {
        assert(&fileNode != nullptr);

        const auto& propList = fileNode.PropertyList();
        const auto count = propList.Count();

        if (count < 1)
            return false;

        if (!AddHeader(propList.Property(0), expectResource))
            return false;

        for (auto i = 0; i < count; i++)
        {
            const auto& prop = propList.Property(i);
            auto propType = prop.Declaration().PropertyType();

            // ordered roughly by expected popularity of each property type
            if (Constants::REPO.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddRepo(prop))
                    return false;
            }
            else if (Constants::PREFIX.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddPrefix(prop))
                    return false;
            }
            else if (Constants::INCLUDE.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddInclude(prop))
                    return false;
            }
            else if (Constants::ALIAS.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddAlias(prop))
                    return false;
            }
            else
            {
                _console.Error() << "Error: Unknown property \"" << propType << "\" in configuration file.\n";
                _console.Error() << FmtPosition(prop);
                return false;
            }
        }

        return true;
    }

    bool FileIR::AddHeader(const PropertyNode& firstNode, bool expectResource)
    {
        auto propType = firstNode.Declaration().PropertyType();

        if (expectResource)
        {
            if (Constants::RESOURCE.IsCaseInsensitiveEqualTo(propType))
            {
                _headerNode = &firstNode;
                _isProfile = false;
                return true;
            }

            _console.Error() << "Error: Resource (included) files must be declared with the \"RESOURCE\" property at the beginning of the file.\n";
            _console.Error() << FmtPosition(firstNode);
            return false;
        }

        if (!Constants::PROFILE.IsCaseInsensitiveEqualTo(propType))
        {
            _console.Error() << "Error: MOT profiles must be declared with the \"PROFILE\" property at the beginning of the file.\n";
            _console.Error() << FmtPosition(firstNode);
            return false;
        }

        _headerNode = &firstNode;
        _isProfile = true;
        return true;
    }

    bool FileIR::AddAlias(const PropertyNode& prop)
    {
        assert(Constants::ALIAS.IsCaseInsensitiveEqualTo(prop.Declaration().PropertyType()));

        //

        return true;
    }

    bool FileIR::AddInclude(const PropertyNode& prop)
    {
        assert(Constants::INCLUDE.IsCaseInsensitiveEqualTo(prop.Declaration().PropertyType()));

        //

        return true;
    }

    bool FileIR::AddPrefix(const PropertyNode& prop)
    {
        assert(Constants::PREFIX.IsCaseInsensitiveEqualTo(prop.Declaration().PropertyType()));

        auto prefixName = prop.Declaration().PropertyName();
        if (MotString::IsEmpty(prefixName))
        {
            _console.Error() << "Error: Prefix property does not have a name\n";
            _console.Error() << FmtPosition(prop);
            return false;
        }

        if (_prefixes.count(prefixName) > 0)
        {
            _console.Error() << "Error: Prefix \"" << prefixName << "\" already exists (note: prefixes are case-insensitive).\n";
            _console.Error() << "    original: " << FmtPosition(_prefixes.at(prefixName)->Node).NoAtPrefix();
            _console.Error() << "    duplicate: " << FmtPosition(prop).NoAtPrefix();
            return false;
        }

        auto ir = std::make_unique<PrefixIR>(PrefixIR(prop, prefixName));
        ir->OverrideMode = OverrideMode::None;

        if (auto valueNode = prop.ValueNode())
        {
            auto value = valueNode->Value();
            if (MotString::IsEmpty(value))
            {
                _console.Error() << "Error: Prefix \"" << prefixName << "\" has an empty value\n";
                _console.Error() << FmtPosition(prop);
                return false;
            }

            ir->PathByEnvironment[Constants::DEFAULT] = value;
        }
        else if (auto block = prop.Block())
        {
            auto foundOverrideProp = false;

            auto& propList = block->PropertyList();
            auto childCount = propList.Count();
            for (auto i = 0u; i < childCount; i++)
            {
                auto& childProp = propList.Property(i);
                auto childPropType = childProp.Declaration().PropertyType();

                if (Constants::ENV.IsCaseInsensitiveEqualTo(childPropType))
                {
                    auto envName = childProp.Declaration().PropertyName();
                    if (MotString::IsEmpty(envName))
                    {
                        _console.Error() << "Error: Environment name is empty in prefix \"" << prefixName << "\"\n";
                        _console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    if (ir->PathByEnvironment.count(envName) > 0)
                    {
                        _console.Error() << "Error: Duplicate environment \"" << envName << "\" in prefix \"" << prefixName << "\"\n";
                        _console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    auto envPath = childProp.HasValue() ? childProp.ValueNode()->Value() : MotString();
                    if (MotString::IsEmpty(envPath))
                    {
                        _console.Error() << "Error: Environment \"" << envName << "\" has no value in prefix \"" << prefixName << "\"\n";
                        _console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    // todo: if value has invalid characters

                    // Everything looks good so far. The path itself might still be invalid, but we can't test that until we resolve the prefixes.
                    ir->PathByEnvironment[envName] = envPath;
                }
                else if (Constants::OVERRIDE.IsCaseInsensitiveEqualTo(childPropType))
                {
                    if (foundOverrideProp)
                    {
                        _console.Error() << "Error: Prefix \"" << prefixName << "\" has more than one override property\n";
                        _console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    foundOverrideProp = true;

                    if (!MotString::IsEmpty(childProp.Declaration().PropertyName()))
                    {
                        _console.Error() << R"(Error: Expected a colon (:) after "override" in prefix ")" << prefixName << "\"\n";
                        _console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    auto overrideValue = childProp.HasValue() ? childProp.ValueNode()->Value() : MotString();
                    if (Constants::MERGE.IsCaseInsensitiveEqualTo(overrideValue))
                    {
                        ir->OverrideMode = OverrideMode::Merge;
                    }
                    else if (Constants::REPLACE.IsCaseInsensitiveEqualTo(overrideValue))
                    {
                        ir->OverrideMode = OverrideMode::Replace;
                    }
                    else if (Constants::NONE.IsCaseInsensitiveEqualTo(overrideValue))
                    {
                        ir->OverrideMode = OverrideMode::None; // technical redundant, but eh
                    }
                    else
                    {
                        _console.Error() << "Error: Override property must be \"merge\", \"replace\", or \"none\"\n";
                        _console.Error() << FmtPosition(childProp);
                        return false;
                    }
                }
                else
                {
                    _console.Error() << "Error: Unknown property type \"" << childPropType << "\" in prefix \"" << prefixName << "\"\n";
                    _console.Error() << FmtPosition(childProp);
                    return false;
                }
            }
        }
        else
        {
            _console.Error() << "Error: Prefix \"" << prefixName << "\" does not define paths for any environment\n";
            _console.Error() << FmtPosition(prop);
            return false;
        }

        _prefixes.emplace(prefixName, std::move(ir));

        return true;
    }

    bool FileIR::AddRepo(const PropertyNode& prop)
    {
        assert(Constants::REPO.IsCaseInsensitiveEqualTo(prop.Declaration().PropertyType()));

        //

        return true;
    }
}
