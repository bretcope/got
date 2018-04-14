#include <cassert>
#include "FileIR.h"
#include "../io/printing/FmtPosition.h"

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

        const MotString REPO("REPO");
        const MotString PREFIX("PREFIX");
        const MotString INCLUDE("INCLUDE");
        const MotString ALIAS("ALIAS");

        for (auto i = 1; i < propCount; i++)
        {
            auto prop = propList->GetProperty(i);
            auto propType = prop->Declaration()->PropertyType();

            // ordered roughly by expected popularity of each property type
            if (REPO.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddRepo(console, prop))
                    return false;
            }
            else if (PREFIX.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddPrefix(console, prop))
                    return false;
            }
            else if (INCLUDE.IsCaseInsensitiveEqualTo(propType))
            {
                if (!AddInclude(console, prop))
                    return false;
            }
            else if (ALIAS.IsCaseInsensitiveEqualTo(propType))
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
            if (MotString("RESOURCE").IsCaseInsensitiveEqualTo(propType))
            {
                _headerNode = firstNode;
                _isProfile = false;
                return true;
            }

            console.Error() << "Error: Resource (included) files must be declared with the \"RESOURCE\" property at the beginning of the file.\n";
            console.Error() << FmtPosition(firstNode);
            return false;
        }

        if (!MotString("PROFILE").IsCaseInsensitiveEqualTo(propType))
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
        assert(MotString("ALIAS").IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        //

        return true;
    }

    bool FileIR::AddInclude(const Console& console, const PropertyNode* prop)
    {
        assert(MotString("INCLUDE").IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        //

        return true;
    }

    bool FileIR::AddPrefix(const Console& console, const PropertyNode* prop)
    {
        assert(MotString("PREFIX").IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

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

        if (prop->HasValue())
        {
            console.Error() << "Error: Prefix \"" << prefixName << "\" must define a block of environments and values instead of a single value.\n";
            console.Error() << FmtPosition(prop);
            return false;
        }

        if (!prop->HasBlock())
        {
            console.Error() << "Error: Prefix \"" << prefixName << "\" is missing value(s).\n";
            console.Error() << FmtPosition(prop);
            return false;
        }

        PrefixIR ir{};
        ir.Node = prop;
        ir.Name = prefixName;
        ir.IsOverride = false;

        MotString OVERRIDE("override");
        MotString ENV("env");
        auto foundOverrideProp = false;

        auto childProps = prop->Block()->PropertyList();
        auto childCount = childProps->Count();
        for (auto i = 0; i < childCount; i++)
        {
            auto childProp = childProps->GetProperty(i);
            auto childPropType = childProp->Declaration()->PropertyType();

            if (ENV.IsCaseInsensitiveEqualTo(childPropType))
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
            else if (OVERRIDE.IsCaseInsensitiveEqualTo(childPropType))
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

        _prefixes[prefixName] = ir;

        return true;
    }

    bool FileIR::AddRepo(const Console& console, const PropertyNode* prop)
    {
        assert(MotString("REPO").IsCaseInsensitiveEqualTo(prop->Declaration()->PropertyType()));

        //

        return true;
    }
}
