#include <cassert>
#include "FileIR.h"
#include "../io/printing/FmtPosition.h"
#include "../text/Constants.h"

namespace mot
{
    FileIR::FileIR(const Console& console, const FileNode* fileNode, bool expectResource)
    {
        _fileNode = fileNode;
        _isValid = TryBuildIR(console, fileNode, expectResource);
    }

    FileIR::~FileIR() = default;

    bool FileIR::IsValid() const
    {
        return _isValid;
    }

    const ByStringPtr<PrefixIR>& FileIR::Prefixes() const
    {
        return _prefixes;
    }

    bool FileIR::TryBuildIR(const Console& console, const FileNode* fileNode, bool expectResource)
    {
        assert(fileNode != nullptr);

        auto propList = fileNode->PropertyList();
        auto propCount = propList->Count();

        if (propCount < 1)
            return false;

        if (!AddHeader(console, propList->GetProperty(0), expectResource))
            return false;

        for (auto i = 1; i < propCount; i++)
        {
            auto prop = propList->GetProperty(i);
            auto propType = prop->Declaration()->PropertyType();

            // ordered roughly by expected popularity of each property type
            if (Constants::REPO.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddRepo(console, prop))
                    return false;
            }
            else if (Constants::PREFIX.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddPrefix(console, prop))
                    return false;
            }
            else if (Constants::INCLUDE.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddInclude(console, prop))
                    return false;
            }
            else if (Constants::ALIAS.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddAlias(console, prop))
                    return false;
            }
            else
            {
                console.Error() << "Error: Unknown property \"" << propType << "\" in configuration file.\n";
                console.Error() << FmtPosition(prop);
                return false;
            }
        }

        return true;
    }

    bool FileIR::AddHeader(const Console& console, const PropertyNode* firstNode, bool expectResource)
    {
        auto propType = firstNode->Declaration()->PropertyType();

        if (expectResource)
        {
            if (Constants::RESOURCE.IsCaseInsensitiveEqualTo(propType))
            {
                _headerNode = firstNode;
                _isProfile = false;
                return true;
            }

            console.Error() << "Error: Resource (included) files must be declared with the \"RESOURCE\" property at the beginning of the file.\n";
            console.Error() << FmtPosition(firstNode);
            return false;
        }

        if (!Constants::PROFILE.IsCaseInsensitiveEqualTo(propType))
        {
            console.Error() << "Error: MOT profiles must be declared with the \"PROFILE\" property at the beginning of the file.\n";
            console.Error() << FmtPosition(firstNode);
            return false;
        }

        _headerNode = firstNode;
        _isProfile = true;
        return true;
    }

    bool FileIR::AddAlias(const Console& console, const PropertyNode* prop)
    {
        assert(Constants::ALIAS.IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        //

        return true;
    }

    bool FileIR::AddInclude(const Console& console, const PropertyNode* prop)
    {
        assert(Constants::INCLUDE.IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        //

        return true;
    }

    bool FileIR::AddPrefix(const Console& console, const PropertyNode* prop)
    {
        assert(Constants::PREFIX.IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        auto prefixName = prop->Declaration()->PropertyName();
        if (MotString::IsEmpty(prefixName))
        {
            console.Error() << "Error: Prefix property does not have a name\n";
            console.Error() << FmtPosition(prop);
            return false;
        }

        if (_prefixes.count(prefixName) > 0)
        {
            console.Error() << "Error: Prefix \"" << prefixName << "\" already exists (note: prefixes are case-insensitive).\n";
            console.Error() << "    original: " << FmtPosition(_prefixes.at(prefixName).Node).NoAtPrefix();
            console.Error() << "    duplicate: " << FmtPosition(prop).NoAtPrefix();
            return false;
        }

        PrefixIR ir{};
        ir.Node = prop;
        ir.Name = prefixName;
        ir.IsOverride = false;

        if (prop->HasValue())
        {
            auto value = prop->ValueNode()->Value();
            if (MotString::IsEmpty(value))
            {
                console.Error() << "Error: Prefix \"" << prefixName << "\" has an empty value\n";
                console.Error() << FmtPosition(prop);
                return false;
            }

            ir.PathByEnvironment[&Constants::DEFAULT] = value;
        }
        else if (prop->HasBlock())
        {
            auto foundOverrideProp = false;

            auto childProps = prop->Block()->PropertyList();
            auto childCount = childProps->Count();
            for (auto i = 0; i < childCount; i++)
            {
                auto childProp = childProps->GetProperty(i);
                auto childPropType = childProp->Declaration()->PropertyType();

                if (Constants::ENV.IsCaseInsensitiveEqualTo(childPropType))
                {
                    auto envName = childProp->Declaration()->PropertyName();
                    if (MotString::IsEmpty(envName))
                    {
                        console.Error() << "Error: Environment name is empty in prefix \"" << prefixName << "\"\n";
                        console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    if (ir.PathByEnvironment.count(envName) > 0)
                    {
                        console.Error() << "Error: Duplicate environment \"" << envName << "\" in prefix \"" << prefixName << "\"\n";
                        console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    auto envPath = childProp->HasValue() ? childProp->ValueNode()->Value() : nullptr;
                    if (MotString::IsEmpty(envPath))
                    {
                        console.Error() << "Error: Environment \"" << envName << "\" has no value in prefix \"" << prefixName << "\"\n";
                        console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    // todo: if value has invalid characters

                    // Everything looks good so far. The path itself might still be invalid, but we can't test that until we resolve the prefixes.
                    ir.PathByEnvironment[envName] = envPath;
                }
                else if (Constants::OVERRIDE.IsCaseInsensitiveEqualTo(childPropType))
                {
                    if (foundOverrideProp)
                    {
                        console.Error() << "Error: Prefix \"" << prefixName << "\" has more than one override property\n";
                        console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    foundOverrideProp = true;

                    if (!MotString::IsEmpty(childProp->Declaration()->PropertyName()))
                    {
                        console.Error() << R"(Error: Expected a colon (:) after "override" in prefix ")" << prefixName << "\"\n";
                        console.Error() << FmtPosition(childProp);
                        return false;
                    }

                    auto overrideValue = childProp->HasValue() ? childProp->ValueNode()->Value() : MotString::Empty();
                    if (MotString("true").IsCaseInsensitiveEqualTo(overrideValue))
                    {
                        ir.IsOverride = true;
                    }
                    else if (MotString("false").IsCaseInsensitiveEqualTo(overrideValue))
                    {
                        ir.IsOverride = false; // technical redundant, but eh
                    }
                    else
                    {
                        console.Error() << "Error: Override property must be \"true\" or \"false\"\n";
                        console.Error() << FmtPosition(childProp);
                        return false;
                    }
                }
                else
                {
                    console.Error() << "Error: Unknown property type \"" << childPropType << "\" in prefix \"" << prefixName << "\"\n";
                    console.Error() << FmtPosition(childProp);
                    return false;
                }
            }
        }
        else
        {
            console.Error() << "Error: Prefix \"" << prefixName << "\" does not define paths for any environment\n";
            console.Error() << FmtPosition(prop);
            return false;
        }

        _prefixes[prefixName] = std::move(ir);

        return true;
    }

    bool FileIR::AddRepo(const Console& console, const PropertyNode* prop)
    {
        assert(Constants::REPO.IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        //

        return true;
    }
}
