#ifndef MOT_MACROS_H
#define MOT_MACROS_H

#define FLAGS_ENUM(TEnum) enum class TEnum : unsigned int;\
inline TEnum operator|(TEnum lhs, TEnum rhs) { return static_cast<TEnum>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs)); }\
inline TEnum& operator|=(TEnum& lhs, TEnum rhs) { lhs = static_cast<TEnum>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs)); return lhs; }\
inline TEnum operator&(TEnum lhs, TEnum rhs) { return static_cast<TEnum>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs)); }\
inline TEnum& operator&=(TEnum& lhs, TEnum rhs) { lhs = static_cast<TEnum>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs)); return lhs; }\
inline TEnum operator^(TEnum lhs, TEnum rhs) { return static_cast<TEnum>(static_cast<unsigned int>(lhs) ^ static_cast<unsigned int>(rhs)); }\
inline TEnum& operator^=(TEnum& lhs, TEnum rhs) { lhs = static_cast<TEnum>(static_cast<unsigned int>(lhs) ^ static_cast<unsigned int>(rhs)); return lhs; }\
inline TEnum operator~(TEnum f) { return static_cast<TEnum>(~static_cast<unsigned int>(f)); }\
enum class TEnum : unsigned int

#endif //MOT_MACROS_H
