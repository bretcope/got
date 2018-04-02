<Query Kind="Program" />

void Main()
{
	Ranges(true);
	//TestCustom();
}

void TestSymmetry()
{
	for (var i = 0; i <= ushort.MaxValue; i++)
	{
		if (i == 0xD800) // surrogate character
		{
			i = 0xE000;
			continue;
		}

		var ch = (char)i;
		if (char.ToLowerInvariant(ch) != ch && char.ToUpperInvariant(char.ToLowerInvariant(ch)) != ch)
		{
			$"Orig:  '{ch}' ({(int)ch})".Dump();
			ch = char.ToLowerInvariant(ch);
			$"Lower: '{ch}' ({(int)ch})".Dump();
			ch = char.ToUpperInvariant(ch);
			$"Upper: '{ch}' ({(int)ch})".Dump();
			throw new Exception();
		}
	}

	"success".Dump();
}

void TestCustom()
{
	for (var i = 0; i <= ushort.MaxValue; i++)
	{
		if (i == 0xD800) // surrogate character
		{
			i = 0xE000;
			continue;
		}

		var ch = (char)i;

		if (char.ToLowerInvariant(ch) != ToLower(ch))
			throw new Exception($"Lower: orig: '{ch}' ({(int)ch}) {char.ToLowerInvariant(ch)} vs {ToLower(ch)}");

		if (char.ToUpperInvariant(ch) != ToUpper(ch))
			throw new Exception($"Upper: orig: '{ch}' ({(int)ch}) {char.ToUpperInvariant(ch)} vs {ToUpper(ch)}");
	}
	
	"success".Dump();
}

void Ranges(bool cpp)
{
	var ranges = new List<Range>();

	for (var i = 0; i <= ushort.MaxValue;)
	{
		if (i == 0xD800) // surrogate character
		{
			i = 0xE000;
			continue;
		}

		var ch = (char)i;
		if (char.ToLowerInvariant(ch) != ch)
		{
			// discover range
			var diff = char.ToLowerInvariant(ch) - i;
			
			if (diff == 1)
			{
				var range = DiscoverEvenOddRange(ch);
				ranges.Add(range);
				i = (int)range.End + 1;
			}
			else
			{
				DiscoverDiffRange(ch, diff, out var upperRange, out var lowerRange);
				ranges.Add(upperRange);
				ranges.Add(lowerRange);
				i = (int)upperRange.End + 1;
			}
		}
		else
		{
			i++;
		}
	}

	EmitCharFunction(ranges, true, cpp);
	EmitCharFunction(ranges, false, cpp);
}

void EmitCharFunction(List<Range> ranges, bool toUpper, bool cpp)
{
	var charT = cpp ? "char32_t" : "char";
	$"{charT} {(cpp ? "Utf8::" : "")}{(toUpper ? "ToUpper" : "ToLower")}({charT} ch)".Dump();
	"{".Dump();
	$"    {(cpp ? "int32_t" : "var")} diff = 0;".Dump();

	var type = toUpper ? RangeType.Lower : RangeType.Upper;
	
	// special case the first range (ASCII)
	var asciiRange = ranges.Where(r => r.Type == RangeType.EvenOdd || r.Type == type).OrderBy(r => r.StartInt).First();
	EmitRangeTree(asciiRange, toUpper);
	
	// Skip(1) because we special-cased ASCII
	var tree = GetRangeTree(ranges.Where(r => r.Type == RangeType.EvenOdd || r.Type == type).OrderBy(r => r.StartInt).Skip(1).ToList(), toUpper);
	EmitRangeTree(tree, toUpper);

	"".Dump();
	"    // no change".Dump();
	"    return ch;".Dump();
	"".Dump();
	"    APPLY_DIFF:".Dump();
	$"   return ({charT})(ch {(toUpper ? "-" : "+")} diff);".Dump();
	"".Dump();
	"    UPPERCASE_IS_EVEN:".Dump();
	if (toUpper)
		$"    return ({charT})(ch & ~1u);".Dump();
	else
		$"    return ({charT})(ch | 1u);".Dump();
	"".Dump();
	"    UPPERCASE_IS_ODD:".Dump();
	if (toUpper)
		$"    return ({charT})((ch - 1) | 1u);".Dump();
	else
		$"    return ({charT})((ch + 1) & ~1u);".Dump();
	"}".Dump();
	"".Dump();
}

IBasicRange GetRangeTree(List<Range> ranges, bool toUpper)
{
	if (ranges.Count == 0)
		throw new Exception();
		
	if (ranges.Count == 1)
		return ranges[0];
		
	var half = ranges.Count / 2;
	var left = GetRangeTree(ranges.Take(half).ToList(), toUpper);
	var right = GetRangeTree(ranges.Skip(half).ToList(), toUpper);
	
	return new ParentRange(left, right);
}

void EmitRangeTree(IBasicRange range, bool toUpper)
{
	$"    if (ch >= {(int)range.Start}u && ch <= {(int)range.End}u) // ['{range.Start}', '{range.End}']".Dump();
	"    {".Dump();
	EmitRange(range, toUpper, "        ");
	"    }".Dump();
	"".Dump();
}

void EmitRange(IBasicRange range, bool toUpper, string indent)
{
	// at this point, we know we're somewhere inside the range
	if (range is Range r)
	{
		if (r.Type == RangeType.EvenOdd)
		{
			if (r.Start % 2 == 0)
			{
				$"{indent}goto UPPERCASE_IS_EVEN;".Dump();
			}
			else
			{
				$"{indent}goto UPPERCASE_IS_ODD;".Dump();
			}
		}
		else
		{
			$"{indent}diff = {r.Diff};".Dump();
			$"{indent}goto APPLY_DIFF;".Dump();
		}
	}
	else if (range is ParentRange p)
	{
		$"{indent}if (ch <= {(int)p.Left.End}u) // ['{p.Left.Start}', '{p.Left.End}']".Dump();
		$"{indent}{{".Dump();
		EmitRange(p.Left, toUpper, indent + "    ");
		$"{indent}}} // ['{p.Left.Start}', '{p.Left.End}']".Dump();
		$"{indent}else if (ch >= {(int)p.Right.Start}u) // ['{p.Right.Start}', '{p.Right.End}']".Dump();
		$"{indent}{{".Dump();
		EmitRange(p.Right, toUpper, indent + "    ");
		$"{indent}}} // ['{p.Right.Start}', '{p.Right.End}']".Dump();
	}
}

void DiscoverDiffRange(char upperStart, int diff, out Range upperRange, out Range lowerRange)
{
	upperRange = new Range() { Type = RangeType.Upper, Start = upperStart, Diff = diff };
	lowerRange = new Range() { Type = RangeType.Lower, Start = (char)(upperStart + diff), Diff = diff };
	var i = (int)upperStart + 1;
	while (char.ToLowerInvariant((char)i) == (char)(i + diff))
	{
		i++;
	}

	upperRange.End = (char)(i - 1);
	lowerRange.End = (char)(i + diff - 1);
	var count = i - upperStart;
	upperRange.Count = count;
	lowerRange.Count = count;
}

Range DiscoverEvenOddRange(char upperStart)
{
	var range = new Range() { Type = RangeType.EvenOdd, Start = upperStart, Diff = 1 };
	var i = (int)upperStart + 2;
	while (char.ToLowerInvariant((char)i) == (char)(i + 1))
	{
		i += 2;
	}

	range.End = (char)(i - 1);
	range.Count = (i - upperStart) / 2;
	return range;
}

enum RangeType
{
	Upper,
	Lower,
	EvenOdd,
}

interface IBasicRange
{
	char Start { get; }
	char End { get; }
}

class Range : IBasicRange
{
	public RangeType Type { get; set; }
	public char Start { get; set; }
	public char End { get; set; }
	public int Count { get; set; }
	public int Diff { get; set; }
	public int StartInt => (int)Start;
	public int EndInt => (int)End;
}

class ParentRange : IBasicRange
{
	public IBasicRange Left { get; }
	public IBasicRange Right { get; }
	
	public char Start => Left.Start;
	public char End => Right?.End ?? Left.End;
	
	public ParentRange(IBasicRange left, IBasicRange right)
	{
		if (left == null || right == null)
			throw new Exception();
			
		Left = left;
		Right = right;
	}
}

char ToUpper(char ch)
{
    var diff = 0;
    if (ch >= 97 && ch <= 122) // ['a', 'z']
    {
        diff = 32;
        goto APPLY_DIFF;
    }

    if (ch >= 224 && ch <= 65370) // ['à', 'ｚ']
    {
        if (ch <= 961) // ['à', 'ρ']
        {
            if (ch <= 543) // ['à', 'ȟ']
            {
                if (ch <= 414) // ['à', 'ƞ']
                {
                    if (ch <= 382) // ['à', 'ž']
                    {
                        if (ch <= 303) // ['à', 'į']
                        {
                            if (ch <= 254) // ['à', 'þ']
                            {
                                if (ch <= 246) // ['à', 'ö']
                                {
                                    diff = 32;
                                    goto APPLY_DIFF;
                                } // ['à', 'ö']
                                else if (ch >= 248) // ['ø', 'þ']
                                {
                                    diff = 32;
                                    goto APPLY_DIFF;
                                } // ['ø', 'þ']
                            } // ['à', 'þ']
                            else if (ch >= 255) // ['ÿ', 'į']
                            {
                                if (ch <= 255) // ['ÿ', 'ÿ']
                                {
                                    diff = -121;
                                    goto APPLY_DIFF;
                                } // ['ÿ', 'ÿ']
                                else if (ch >= 256) // ['Ā', 'į']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ā', 'į']
                            } // ['ÿ', 'į']
                        } // ['à', 'į']
                        else if (ch >= 306) // ['Ĳ', 'ž']
                        {
                            if (ch <= 328) // ['Ĳ', 'ň']
                            {
                                if (ch <= 311) // ['Ĳ', 'ķ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ĳ', 'ķ']
                                else if (ch >= 313) // ['Ĺ', 'ň']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ĺ', 'ň']
                            } // ['Ĳ', 'ň']
                            else if (ch >= 330) // ['Ŋ', 'ž']
                            {
                                if (ch <= 375) // ['Ŋ', 'ŷ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ŋ', 'ŷ']
                                else if (ch >= 377) // ['Ź', 'ž']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ź', 'ž']
                            } // ['Ŋ', 'ž']
                        } // ['Ĳ', 'ž']
                    } // ['à', 'ž']
                    else if (ch >= 384) // ['ƀ', 'ƞ']
                    {
                        if (ch <= 396) // ['ƀ', 'ƌ']
                        {
                            if (ch <= 389) // ['ƀ', 'ƅ']
                            {
                                if (ch <= 384) // ['ƀ', 'ƀ']
                                {
                                    diff = -195;
                                    goto APPLY_DIFF;
                                } // ['ƀ', 'ƀ']
                                else if (ch >= 386) // ['Ƃ', 'ƅ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ƃ', 'ƅ']
                            } // ['ƀ', 'ƅ']
                            else if (ch >= 391) // ['Ƈ', 'ƌ']
                            {
                                if (ch <= 392) // ['Ƈ', 'ƈ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ƈ', 'ƈ']
                                else if (ch >= 395) // ['Ƌ', 'ƌ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ƌ', 'ƌ']
                            } // ['Ƈ', 'ƌ']
                        } // ['ƀ', 'ƌ']
                        else if (ch >= 401) // ['Ƒ', 'ƞ']
                        {
                            if (ch <= 405) // ['Ƒ', 'ƕ']
                            {
                                if (ch <= 402) // ['Ƒ', 'ƒ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ƒ', 'ƒ']
                                else if (ch >= 405) // ['ƕ', 'ƕ']
                                {
                                    diff = -97;
                                    goto APPLY_DIFF;
                                } // ['ƕ', 'ƕ']
                            } // ['Ƒ', 'ƕ']
                            else if (ch >= 408) // ['Ƙ', 'ƞ']
                            {
                                if (ch <= 409) // ['Ƙ', 'ƙ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ƙ', 'ƙ']
                                else if (ch >= 410) // ['ƚ', 'ƞ']
                                {
                                    if (ch <= 410) // ['ƚ', 'ƚ']
                                    {
                                        diff = -163;
                                        goto APPLY_DIFF;
                                    } // ['ƚ', 'ƚ']
                                    else if (ch >= 414) // ['ƞ', 'ƞ']
                                    {
                                        diff = -130;
                                        goto APPLY_DIFF;
                                    } // ['ƞ', 'ƞ']
                                } // ['ƚ', 'ƞ']
                            } // ['Ƙ', 'ƞ']
                        } // ['Ƒ', 'ƞ']
                    } // ['ƀ', 'ƞ']
                } // ['à', 'ƞ']
                else if (ch >= 416) // ['Ơ', 'ȟ']
                {
                    if (ch <= 447) // ['Ơ', 'ƿ']
                    {
                        if (ch <= 432) // ['Ơ', 'ư']
                        {
                            if (ch <= 424) // ['Ơ', 'ƨ']
                            {
                                if (ch <= 421) // ['Ơ', 'ƥ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ơ', 'ƥ']
                                else if (ch >= 423) // ['Ƨ', 'ƨ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ƨ', 'ƨ']
                            } // ['Ơ', 'ƨ']
                            else if (ch >= 428) // ['Ƭ', 'ư']
                            {
                                if (ch <= 429) // ['Ƭ', 'ƭ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ƭ', 'ƭ']
                                else if (ch >= 431) // ['Ư', 'ư']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ư', 'ư']
                            } // ['Ƭ', 'ư']
                        } // ['Ơ', 'ư']
                        else if (ch >= 435) // ['Ƴ', 'ƿ']
                        {
                            if (ch <= 441) // ['Ƴ', 'ƹ']
                            {
                                if (ch <= 438) // ['Ƴ', 'ƶ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ƴ', 'ƶ']
                                else if (ch >= 440) // ['Ƹ', 'ƹ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ƹ', 'ƹ']
                            } // ['Ƴ', 'ƹ']
                            else if (ch >= 444) // ['Ƽ', 'ƿ']
                            {
                                if (ch <= 445) // ['Ƽ', 'ƽ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ƽ', 'ƽ']
                                else if (ch >= 447) // ['ƿ', 'ƿ']
                                {
                                    diff = -56;
                                    goto APPLY_DIFF;
                                } // ['ƿ', 'ƿ']
                            } // ['Ƽ', 'ƿ']
                        } // ['Ƴ', 'ƿ']
                    } // ['Ơ', 'ƿ']
                    else if (ch >= 454) // ['ǆ', 'ȟ']
                    {
                        if (ch <= 476) // ['ǆ', 'ǜ']
                        {
                            if (ch <= 457) // ['ǆ', 'ǉ']
                            {
                                if (ch <= 454) // ['ǆ', 'ǆ']
                                {
                                    diff = 2;
                                    goto APPLY_DIFF;
                                } // ['ǆ', 'ǆ']
                                else if (ch >= 457) // ['ǉ', 'ǉ']
                                {
                                    diff = 2;
                                    goto APPLY_DIFF;
                                } // ['ǉ', 'ǉ']
                            } // ['ǆ', 'ǉ']
                            else if (ch >= 460) // ['ǌ', 'ǜ']
                            {
                                if (ch <= 460) // ['ǌ', 'ǌ']
                                {
                                    diff = 2;
                                    goto APPLY_DIFF;
                                } // ['ǌ', 'ǌ']
                                else if (ch >= 461) // ['Ǎ', 'ǜ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ǎ', 'ǜ']
                            } // ['ǌ', 'ǜ']
                        } // ['ǆ', 'ǜ']
                        else if (ch >= 477) // ['ǝ', 'ȟ']
                        {
                            if (ch <= 495) // ['ǝ', 'ǯ']
                            {
                                if (ch <= 477) // ['ǝ', 'ǝ']
                                {
                                    diff = 79;
                                    goto APPLY_DIFF;
                                } // ['ǝ', 'ǝ']
                                else if (ch >= 478) // ['Ǟ', 'ǯ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ǟ', 'ǯ']
                            } // ['ǝ', 'ǯ']
                            else if (ch >= 499) // ['ǳ', 'ȟ']
                            {
                                if (ch <= 499) // ['ǳ', 'ǳ']
                                {
                                    diff = 2;
                                    goto APPLY_DIFF;
                                } // ['ǳ', 'ǳ']
                                else if (ch >= 500) // ['Ǵ', 'ȟ']
                                {
                                    if (ch <= 501) // ['Ǵ', 'ǵ']
                                    {
                                        goto UPPERCASE_IS_EVEN;
                                    } // ['Ǵ', 'ǵ']
                                    else if (ch >= 504) // ['Ǹ', 'ȟ']
                                    {
                                        goto UPPERCASE_IS_EVEN;
                                    } // ['Ǹ', 'ȟ']
                                } // ['Ǵ', 'ȟ']
                            } // ['ǳ', 'ȟ']
                        } // ['ǝ', 'ȟ']
                    } // ['ǆ', 'ȟ']
                } // ['Ơ', 'ȟ']
            } // ['à', 'ȟ']
            else if (ch >= 546) // ['Ȣ', 'ρ']
            {
                if (ch <= 623) // ['Ȣ', 'ɯ']
                {
                    if (ch <= 596) // ['Ȣ', 'ɔ']
                    {
                        if (ch <= 591) // ['Ȣ', 'ɏ']
                        {
                            if (ch <= 572) // ['Ȣ', 'ȼ']
                            {
                                if (ch <= 563) // ['Ȣ', 'ȳ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ȣ', 'ȳ']
                                else if (ch >= 571) // ['Ȼ', 'ȼ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ȼ', 'ȼ']
                            } // ['Ȣ', 'ȼ']
                            else if (ch >= 577) // ['Ɂ', 'ɏ']
                            {
                                if (ch <= 578) // ['Ɂ', 'ɂ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ɂ', 'ɂ']
                                else if (ch >= 582) // ['Ɇ', 'ɏ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ɇ', 'ɏ']
                            } // ['Ɂ', 'ɏ']
                        } // ['Ȣ', 'ɏ']
                        else if (ch >= 592) // ['ɐ', 'ɔ']
                        {
                            if (ch <= 593) // ['ɐ', 'ɑ']
                            {
                                if (ch <= 592) // ['ɐ', 'ɐ']
                                {
                                    diff = -10783;
                                    goto APPLY_DIFF;
                                } // ['ɐ', 'ɐ']
                                else if (ch >= 593) // ['ɑ', 'ɑ']
                                {
                                    diff = -10780;
                                    goto APPLY_DIFF;
                                } // ['ɑ', 'ɑ']
                            } // ['ɐ', 'ɑ']
                            else if (ch >= 595) // ['ɓ', 'ɔ']
                            {
                                if (ch <= 595) // ['ɓ', 'ɓ']
                                {
                                    diff = 210;
                                    goto APPLY_DIFF;
                                } // ['ɓ', 'ɓ']
                                else if (ch >= 596) // ['ɔ', 'ɔ']
                                {
                                    diff = 206;
                                    goto APPLY_DIFF;
                                } // ['ɔ', 'ɔ']
                            } // ['ɓ', 'ɔ']
                        } // ['ɐ', 'ɔ']
                    } // ['Ȣ', 'ɔ']
                    else if (ch >= 598) // ['ɖ', 'ɯ']
                    {
                        if (ch <= 608) // ['ɖ', 'ɠ']
                        {
                            if (ch <= 601) // ['ɖ', 'ə']
                            {
                                if (ch <= 599) // ['ɖ', 'ɗ']
                                {
                                    diff = 205;
                                    goto APPLY_DIFF;
                                } // ['ɖ', 'ɗ']
                                else if (ch >= 601) // ['ə', 'ə']
                                {
                                    diff = 202;
                                    goto APPLY_DIFF;
                                } // ['ə', 'ə']
                            } // ['ɖ', 'ə']
                            else if (ch >= 603) // ['ɛ', 'ɠ']
                            {
                                if (ch <= 603) // ['ɛ', 'ɛ']
                                {
                                    diff = 203;
                                    goto APPLY_DIFF;
                                } // ['ɛ', 'ɛ']
                                else if (ch >= 608) // ['ɠ', 'ɠ']
                                {
                                    diff = 205;
                                    goto APPLY_DIFF;
                                } // ['ɠ', 'ɠ']
                            } // ['ɛ', 'ɠ']
                        } // ['ɖ', 'ɠ']
                        else if (ch >= 611) // ['ɣ', 'ɯ']
                        {
                            if (ch <= 616) // ['ɣ', 'ɨ']
                            {
                                if (ch <= 611) // ['ɣ', 'ɣ']
                                {
                                    diff = 207;
                                    goto APPLY_DIFF;
                                } // ['ɣ', 'ɣ']
                                else if (ch >= 616) // ['ɨ', 'ɨ']
                                {
                                    diff = 209;
                                    goto APPLY_DIFF;
                                } // ['ɨ', 'ɨ']
                            } // ['ɣ', 'ɨ']
                            else if (ch >= 617) // ['ɩ', 'ɯ']
                            {
                                if (ch <= 617) // ['ɩ', 'ɩ']
                                {
                                    diff = 211;
                                    goto APPLY_DIFF;
                                } // ['ɩ', 'ɩ']
                                else if (ch >= 619) // ['ɫ', 'ɯ']
                                {
                                    if (ch <= 619) // ['ɫ', 'ɫ']
                                    {
                                        diff = -10743;
                                        goto APPLY_DIFF;
                                    } // ['ɫ', 'ɫ']
                                    else if (ch >= 623) // ['ɯ', 'ɯ']
                                    {
                                        diff = 211;
                                        goto APPLY_DIFF;
                                    } // ['ɯ', 'ɯ']
                                } // ['ɫ', 'ɯ']
                            } // ['ɩ', 'ɯ']
                        } // ['ɣ', 'ɯ']
                    } // ['ɖ', 'ɯ']
                } // ['Ȣ', 'ɯ']
                else if (ch >= 625) // ['ɱ', 'ρ']
                {
                    if (ch <= 649) // ['ɱ', 'ʉ']
                    {
                        if (ch <= 637) // ['ɱ', 'ɽ']
                        {
                            if (ch <= 626) // ['ɱ', 'ɲ']
                            {
                                if (ch <= 625) // ['ɱ', 'ɱ']
                                {
                                    diff = -10749;
                                    goto APPLY_DIFF;
                                } // ['ɱ', 'ɱ']
                                else if (ch >= 626) // ['ɲ', 'ɲ']
                                {
                                    diff = 213;
                                    goto APPLY_DIFF;
                                } // ['ɲ', 'ɲ']
                            } // ['ɱ', 'ɲ']
                            else if (ch >= 629) // ['ɵ', 'ɽ']
                            {
                                if (ch <= 629) // ['ɵ', 'ɵ']
                                {
                                    diff = 214;
                                    goto APPLY_DIFF;
                                } // ['ɵ', 'ɵ']
                                else if (ch >= 637) // ['ɽ', 'ɽ']
                                {
                                    diff = -10727;
                                    goto APPLY_DIFF;
                                } // ['ɽ', 'ɽ']
                            } // ['ɵ', 'ɽ']
                        } // ['ɱ', 'ɽ']
                        else if (ch >= 640) // ['ʀ', 'ʉ']
                        {
                            if (ch <= 643) // ['ʀ', 'ʃ']
                            {
                                if (ch <= 640) // ['ʀ', 'ʀ']
                                {
                                    diff = 218;
                                    goto APPLY_DIFF;
                                } // ['ʀ', 'ʀ']
                                else if (ch >= 643) // ['ʃ', 'ʃ']
                                {
                                    diff = 218;
                                    goto APPLY_DIFF;
                                } // ['ʃ', 'ʃ']
                            } // ['ʀ', 'ʃ']
                            else if (ch >= 648) // ['ʈ', 'ʉ']
                            {
                                if (ch <= 648) // ['ʈ', 'ʈ']
                                {
                                    diff = 218;
                                    goto APPLY_DIFF;
                                } // ['ʈ', 'ʈ']
                                else if (ch >= 649) // ['ʉ', 'ʉ']
                                {
                                    diff = 69;
                                    goto APPLY_DIFF;
                                } // ['ʉ', 'ʉ']
                            } // ['ʈ', 'ʉ']
                        } // ['ʀ', 'ʉ']
                    } // ['ɱ', 'ʉ']
                    else if (ch >= 650) // ['ʊ', 'ρ']
                    {
                        if (ch <= 883) // ['ʊ', 'ͳ']
                        {
                            if (ch <= 652) // ['ʊ', 'ʌ']
                            {
                                if (ch <= 651) // ['ʊ', 'ʋ']
                                {
                                    diff = 217;
                                    goto APPLY_DIFF;
                                } // ['ʊ', 'ʋ']
                                else if (ch >= 652) // ['ʌ', 'ʌ']
                                {
                                    diff = 71;
                                    goto APPLY_DIFF;
                                } // ['ʌ', 'ʌ']
                            } // ['ʊ', 'ʌ']
                            else if (ch >= 658) // ['ʒ', 'ͳ']
                            {
                                if (ch <= 658) // ['ʒ', 'ʒ']
                                {
                                    diff = 219;
                                    goto APPLY_DIFF;
                                } // ['ʒ', 'ʒ']
                                else if (ch >= 880) // ['Ͱ', 'ͳ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ͱ', 'ͳ']
                            } // ['ʒ', 'ͳ']
                        } // ['ʊ', 'ͳ']
                        else if (ch >= 886) // ['Ͷ', 'ρ']
                        {
                            if (ch <= 893) // ['Ͷ', 'ͽ']
                            {
                                if (ch <= 887) // ['Ͷ', 'ͷ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ͷ', 'ͷ']
                                else if (ch >= 891) // ['ͻ', 'ͽ']
                                {
                                    diff = -130;
                                    goto APPLY_DIFF;
                                } // ['ͻ', 'ͽ']
                            } // ['Ͷ', 'ͽ']
                            else if (ch >= 940) // ['ά', 'ρ']
                            {
                                if (ch <= 940) // ['ά', 'ά']
                                {
                                    diff = 38;
                                    goto APPLY_DIFF;
                                } // ['ά', 'ά']
                                else if (ch >= 941) // ['έ', 'ρ']
                                {
                                    if (ch <= 943) // ['έ', 'ί']
                                    {
                                        diff = 37;
                                        goto APPLY_DIFF;
                                    } // ['έ', 'ί']
                                    else if (ch >= 945) // ['α', 'ρ']
                                    {
                                        diff = 32;
                                        goto APPLY_DIFF;
                                    } // ['α', 'ρ']
                                } // ['έ', 'ρ']
                            } // ['ά', 'ρ']
                        } // ['Ͷ', 'ρ']
                    } // ['ʊ', 'ρ']
                } // ['ɱ', 'ρ']
            } // ['Ȣ', 'ρ']
        } // ['à', 'ρ']
        else if (ch >= 963) // ['σ', 'ｚ']
        {
            if (ch <= 8057) // ['σ', 'ό']
            {
                if (ch <= 7545) // ['σ', 'ᵹ']
                {
                    if (ch <= 1019) // ['σ', 'ϻ']
                    {
                        if (ch <= 983) // ['σ', 'ϗ']
                        {
                            if (ch <= 972) // ['σ', 'ό']
                            {
                                if (ch <= 971) // ['σ', 'ϋ']
                                {
                                    diff = 32;
                                    goto APPLY_DIFF;
                                } // ['σ', 'ϋ']
                                else if (ch >= 972) // ['ό', 'ό']
                                {
                                    diff = 64;
                                    goto APPLY_DIFF;
                                } // ['ό', 'ό']
                            } // ['σ', 'ό']
                            else if (ch >= 973) // ['ύ', 'ϗ']
                            {
                                if (ch <= 974) // ['ύ', 'ώ']
                                {
                                    diff = 63;
                                    goto APPLY_DIFF;
                                } // ['ύ', 'ώ']
                                else if (ch >= 983) // ['ϗ', 'ϗ']
                                {
                                    diff = 8;
                                    goto APPLY_DIFF;
                                } // ['ϗ', 'ϗ']
                            } // ['ύ', 'ϗ']
                        } // ['σ', 'ϗ']
                        else if (ch >= 984) // ['Ϙ', 'ϻ']
                        {
                            if (ch <= 1010) // ['Ϙ', 'ϲ']
                            {
                                if (ch <= 1007) // ['Ϙ', 'ϯ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ϙ', 'ϯ']
                                else if (ch >= 1010) // ['ϲ', 'ϲ']
                                {
                                    diff = -7;
                                    goto APPLY_DIFF;
                                } // ['ϲ', 'ϲ']
                            } // ['Ϙ', 'ϲ']
                            else if (ch >= 1015) // ['Ϸ', 'ϻ']
                            {
                                if (ch <= 1016) // ['Ϸ', 'ϸ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ϸ', 'ϸ']
                                else if (ch >= 1018) // ['Ϻ', 'ϻ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ϻ', 'ϻ']
                            } // ['Ϸ', 'ϻ']
                        } // ['Ϙ', 'ϻ']
                    } // ['σ', 'ϻ']
                    else if (ch >= 1072) // ['а', 'ᵹ']
                    {
                        if (ch <= 1215) // ['а', 'ҿ']
                        {
                            if (ch <= 1119) // ['а', 'џ']
                            {
                                if (ch <= 1103) // ['а', 'я']
                                {
                                    diff = 32;
                                    goto APPLY_DIFF;
                                } // ['а', 'я']
                                else if (ch >= 1104) // ['ѐ', 'џ']
                                {
                                    diff = 80;
                                    goto APPLY_DIFF;
                                } // ['ѐ', 'џ']
                            } // ['а', 'џ']
                            else if (ch >= 1120) // ['Ѡ', 'ҿ']
                            {
                                if (ch <= 1153) // ['Ѡ', 'ҁ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ѡ', 'ҁ']
                                else if (ch >= 1162) // ['Ҋ', 'ҿ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ҋ', 'ҿ']
                            } // ['Ѡ', 'ҿ']
                        } // ['а', 'ҿ']
                        else if (ch >= 1217) // ['Ӂ', 'ᵹ']
                        {
                            if (ch <= 1231) // ['Ӂ', 'ӏ']
                            {
                                if (ch <= 1230) // ['Ӂ', 'ӎ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ӂ', 'ӎ']
                                else if (ch >= 1231) // ['ӏ', 'ӏ']
                                {
                                    diff = 15;
                                    goto APPLY_DIFF;
                                } // ['ӏ', 'ӏ']
                            } // ['Ӂ', 'ӏ']
                            else if (ch >= 1232) // ['Ӑ', 'ᵹ']
                            {
                                if (ch <= 1315) // ['Ӑ', 'ԣ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ӑ', 'ԣ']
                                else if (ch >= 1377) // ['ա', 'ᵹ']
                                {
                                    if (ch <= 1414) // ['ա', 'ֆ']
                                    {
                                        diff = 48;
                                        goto APPLY_DIFF;
                                    } // ['ա', 'ֆ']
                                    else if (ch >= 7545) // ['ᵹ', 'ᵹ']
                                    {
                                        diff = -35332;
                                        goto APPLY_DIFF;
                                    } // ['ᵹ', 'ᵹ']
                                } // ['ա', 'ᵹ']
                            } // ['Ӑ', 'ᵹ']
                        } // ['Ӂ', 'ᵹ']
                    } // ['а', 'ᵹ']
                } // ['σ', 'ᵹ']
                else if (ch >= 7549) // ['ᵽ', 'ό']
                {
                    if (ch <= 8005) // ['ᵽ', 'ὅ']
                    {
                        if (ch <= 7943) // ['ᵽ', 'ἇ']
                        {
                            if (ch <= 7829) // ['ᵽ', 'ẕ']
                            {
                                if (ch <= 7549) // ['ᵽ', 'ᵽ']
                                {
                                    diff = -3814;
                                    goto APPLY_DIFF;
                                } // ['ᵽ', 'ᵽ']
                                else if (ch >= 7680) // ['Ḁ', 'ẕ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ḁ', 'ẕ']
                            } // ['ᵽ', 'ẕ']
                            else if (ch >= 7840) // ['Ạ', 'ἇ']
                            {
                                if (ch <= 7935) // ['Ạ', 'ỿ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ạ', 'ỿ']
                                else if (ch >= 7936) // ['ἀ', 'ἇ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ἀ', 'ἇ']
                            } // ['Ạ', 'ἇ']
                        } // ['ᵽ', 'ἇ']
                        else if (ch >= 7952) // ['ἐ', 'ὅ']
                        {
                            if (ch <= 7975) // ['ἐ', 'ἧ']
                            {
                                if (ch <= 7957) // ['ἐ', 'ἕ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ἐ', 'ἕ']
                                else if (ch >= 7968) // ['ἠ', 'ἧ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ἠ', 'ἧ']
                            } // ['ἐ', 'ἧ']
                            else if (ch >= 7984) // ['ἰ', 'ὅ']
                            {
                                if (ch <= 7991) // ['ἰ', 'ἷ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ἰ', 'ἷ']
                                else if (ch >= 8000) // ['ὀ', 'ὅ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ὀ', 'ὅ']
                            } // ['ἰ', 'ὅ']
                        } // ['ἐ', 'ὅ']
                    } // ['ᵽ', 'ὅ']
                    else if (ch >= 8017) // ['ὑ', 'ό']
                    {
                        if (ch <= 8023) // ['ὑ', 'ὗ']
                        {
                            if (ch <= 8019) // ['ὑ', 'ὓ']
                            {
                                if (ch <= 8017) // ['ὑ', 'ὑ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ὑ', 'ὑ']
                                else if (ch >= 8019) // ['ὓ', 'ὓ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ὓ', 'ὓ']
                            } // ['ὑ', 'ὓ']
                            else if (ch >= 8021) // ['ὕ', 'ὗ']
                            {
                                if (ch <= 8021) // ['ὕ', 'ὕ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ὕ', 'ὕ']
                                else if (ch >= 8023) // ['ὗ', 'ὗ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ὗ', 'ὗ']
                            } // ['ὕ', 'ὗ']
                        } // ['ὑ', 'ὗ']
                        else if (ch >= 8032) // ['ὠ', 'ό']
                        {
                            if (ch <= 8049) // ['ὠ', 'ά']
                            {
                                if (ch <= 8039) // ['ὠ', 'ὧ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ὠ', 'ὧ']
                                else if (ch >= 8048) // ['ὰ', 'ά']
                                {
                                    diff = -74;
                                    goto APPLY_DIFF;
                                } // ['ὰ', 'ά']
                            } // ['ὠ', 'ά']
                            else if (ch >= 8050) // ['ὲ', 'ό']
                            {
                                if (ch <= 8053) // ['ὲ', 'ή']
                                {
                                    diff = -86;
                                    goto APPLY_DIFF;
                                } // ['ὲ', 'ή']
                                else if (ch >= 8054) // ['ὶ', 'ό']
                                {
                                    if (ch <= 8055) // ['ὶ', 'ί']
                                    {
                                        diff = -100;
                                        goto APPLY_DIFF;
                                    } // ['ὶ', 'ί']
                                    else if (ch >= 8056) // ['ὸ', 'ό']
                                    {
                                        diff = -128;
                                        goto APPLY_DIFF;
                                    } // ['ὸ', 'ό']
                                } // ['ὶ', 'ό']
                            } // ['ὲ', 'ό']
                        } // ['ὠ', 'ό']
                    } // ['ὑ', 'ό']
                } // ['ᵽ', 'ό']
            } // ['σ', 'ό']
            else if (ch >= 8058) // ['ὺ', 'ｚ']
            {
                if (ch <= 11358) // ['ὺ', 'ⱞ']
                {
                    if (ch <= 8131) // ['ὺ', 'ῃ']
                    {
                        if (ch <= 8087) // ['ὺ', 'ᾗ']
                        {
                            if (ch <= 8061) // ['ὺ', 'ώ']
                            {
                                if (ch <= 8059) // ['ὺ', 'ύ']
                                {
                                    diff = -112;
                                    goto APPLY_DIFF;
                                } // ['ὺ', 'ύ']
                                else if (ch >= 8060) // ['ὼ', 'ώ']
                                {
                                    diff = -126;
                                    goto APPLY_DIFF;
                                } // ['ὼ', 'ώ']
                            } // ['ὺ', 'ώ']
                            else if (ch >= 8064) // ['ᾀ', 'ᾗ']
                            {
                                if (ch <= 8071) // ['ᾀ', 'ᾇ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ᾀ', 'ᾇ']
                                else if (ch >= 8080) // ['ᾐ', 'ᾗ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ᾐ', 'ᾗ']
                            } // ['ᾀ', 'ᾗ']
                        } // ['ὺ', 'ᾗ']
                        else if (ch >= 8096) // ['ᾠ', 'ῃ']
                        {
                            if (ch <= 8113) // ['ᾠ', 'ᾱ']
                            {
                                if (ch <= 8103) // ['ᾠ', 'ᾧ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ᾠ', 'ᾧ']
                                else if (ch >= 8112) // ['ᾰ', 'ᾱ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ᾰ', 'ᾱ']
                            } // ['ᾠ', 'ᾱ']
                            else if (ch >= 8115) // ['ᾳ', 'ῃ']
                            {
                                if (ch <= 8115) // ['ᾳ', 'ᾳ']
                                {
                                    diff = -9;
                                    goto APPLY_DIFF;
                                } // ['ᾳ', 'ᾳ']
                                else if (ch >= 8131) // ['ῃ', 'ῃ']
                                {
                                    diff = -9;
                                    goto APPLY_DIFF;
                                } // ['ῃ', 'ῃ']
                            } // ['ᾳ', 'ῃ']
                        } // ['ᾠ', 'ῃ']
                    } // ['ὺ', 'ῃ']
                    else if (ch >= 8144) // ['ῐ', 'ⱞ']
                    {
                        if (ch <= 8179) // ['ῐ', 'ῳ']
                        {
                            if (ch <= 8161) // ['ῐ', 'ῡ']
                            {
                                if (ch <= 8145) // ['ῐ', 'ῑ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ῐ', 'ῑ']
                                else if (ch >= 8160) // ['ῠ', 'ῡ']
                                {
                                    diff = -8;
                                    goto APPLY_DIFF;
                                } // ['ῠ', 'ῡ']
                            } // ['ῐ', 'ῡ']
                            else if (ch >= 8165) // ['ῥ', 'ῳ']
                            {
                                if (ch <= 8165) // ['ῥ', 'ῥ']
                                {
                                    diff = -7;
                                    goto APPLY_DIFF;
                                } // ['ῥ', 'ῥ']
                                else if (ch >= 8179) // ['ῳ', 'ῳ']
                                {
                                    diff = -9;
                                    goto APPLY_DIFF;
                                } // ['ῳ', 'ῳ']
                            } // ['ῥ', 'ῳ']
                        } // ['ῐ', 'ῳ']
                        else if (ch >= 8526) // ['ⅎ', 'ⱞ']
                        {
                            if (ch <= 8575) // ['ⅎ', 'ⅿ']
                            {
                                if (ch <= 8526) // ['ⅎ', 'ⅎ']
                                {
                                    diff = 28;
                                    goto APPLY_DIFF;
                                } // ['ⅎ', 'ⅎ']
                                else if (ch >= 8560) // ['ⅰ', 'ⅿ']
                                {
                                    diff = 16;
                                    goto APPLY_DIFF;
                                } // ['ⅰ', 'ⅿ']
                            } // ['ⅎ', 'ⅿ']
                            else if (ch >= 8579) // ['Ↄ', 'ⱞ']
                            {
                                if (ch <= 8580) // ['Ↄ', 'ↄ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ↄ', 'ↄ']
                                else if (ch >= 9424) // ['ⓐ', 'ⱞ']
                                {
                                    if (ch <= 9449) // ['ⓐ', 'ⓩ']
                                    {
                                        diff = 26;
                                        goto APPLY_DIFF;
                                    } // ['ⓐ', 'ⓩ']
                                    else if (ch >= 11312) // ['ⰰ', 'ⱞ']
                                    {
                                        diff = 48;
                                        goto APPLY_DIFF;
                                    } // ['ⰰ', 'ⱞ']
                                } // ['ⓐ', 'ⱞ']
                            } // ['Ↄ', 'ⱞ']
                        } // ['ⅎ', 'ⱞ']
                    } // ['ῐ', 'ⱞ']
                } // ['ὺ', 'ⱞ']
                else if (ch >= 11360) // ['Ⱡ', 'ｚ']
                {
                    if (ch <= 11557) // ['Ⱡ', 'ⴥ']
                    {
                        if (ch <= 11372) // ['Ⱡ', 'ⱬ']
                        {
                            if (ch <= 11365) // ['Ⱡ', 'ⱥ']
                            {
                                if (ch <= 11361) // ['Ⱡ', 'ⱡ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ⱡ', 'ⱡ']
                                else if (ch >= 11365) // ['ⱥ', 'ⱥ']
                                {
                                    diff = 10795;
                                    goto APPLY_DIFF;
                                } // ['ⱥ', 'ⱥ']
                            } // ['Ⱡ', 'ⱥ']
                            else if (ch >= 11366) // ['ⱦ', 'ⱬ']
                            {
                                if (ch <= 11366) // ['ⱦ', 'ⱦ']
                                {
                                    diff = 10792;
                                    goto APPLY_DIFF;
                                } // ['ⱦ', 'ⱦ']
                                else if (ch >= 11367) // ['Ⱨ', 'ⱬ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ⱨ', 'ⱬ']
                            } // ['ⱦ', 'ⱬ']
                        } // ['Ⱡ', 'ⱬ']
                        else if (ch >= 11378) // ['Ⱳ', 'ⴥ']
                        {
                            if (ch <= 11382) // ['Ⱳ', 'ⱶ']
                            {
                                if (ch <= 11379) // ['Ⱳ', 'ⱳ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ⱳ', 'ⱳ']
                                else if (ch >= 11381) // ['Ⱶ', 'ⱶ']
                                {
                                    goto UPPERCASE_IS_ODD;
                                } // ['Ⱶ', 'ⱶ']
                            } // ['Ⱳ', 'ⱶ']
                            else if (ch >= 11392) // ['Ⲁ', 'ⴥ']
                            {
                                if (ch <= 11491) // ['Ⲁ', 'ⳣ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ⲁ', 'ⳣ']
                                else if (ch >= 11520) // ['ⴀ', 'ⴥ']
                                {
                                    diff = 7264;
                                    goto APPLY_DIFF;
                                } // ['ⴀ', 'ⴥ']
                            } // ['Ⲁ', 'ⴥ']
                        } // ['Ⱳ', 'ⴥ']
                    } // ['Ⱡ', 'ⴥ']
                    else if (ch >= 42560) // ['Ꙁ', 'ｚ']
                    {
                        if (ch <= 42799) // ['Ꙁ', 'ꜯ']
                        {
                            if (ch <= 42605) // ['Ꙁ', 'ꙭ']
                            {
                                if (ch <= 42591) // ['Ꙁ', 'ꙟ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ꙁ', 'ꙟ']
                                else if (ch >= 42594) // ['Ꙣ', 'ꙭ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ꙣ', 'ꙭ']
                            } // ['Ꙁ', 'ꙭ']
                            else if (ch >= 42624) // ['Ꚁ', 'ꜯ']
                            {
                                if (ch <= 42647) // ['Ꚁ', 'ꚗ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ꚁ', 'ꚗ']
                                else if (ch >= 42786) // ['Ꜣ', 'ꜯ']
                                {
                                    goto UPPERCASE_IS_EVEN;
                                } // ['Ꜣ', 'ꜯ']
                            } // ['Ꚁ', 'ꜯ']
						} // ['Ꙁ', 'ꜯ']
						else if (ch >= 42802) // ['Ꜳ', 'ｚ']
						{
							if (ch <= 42876) // ['Ꜳ', 'ꝼ']
							{
								if (ch <= 42863) // ['Ꜳ', 'ꝯ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꜳ', 'ꝯ']
								else if (ch >= 42873) // ['Ꝺ', 'ꝼ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ꝺ', 'ꝼ']
							} // ['Ꜳ', 'ꝼ']
							else if (ch >= 42878) // ['Ꝿ', 'ｚ']
							{
								if (ch <= 42887) // ['Ꝿ', 'ꞇ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꝿ', 'ꞇ']
								else if (ch >= 42891) // ['Ꞌ', 'ｚ']
								{
									if (ch <= 42892) // ['Ꞌ', 'ꞌ']
									{
										goto UPPERCASE_IS_ODD;
									} // ['Ꞌ', 'ꞌ']
									else if (ch >= 65345) // ['ａ', 'ｚ']
									{
										diff = 32;
										goto APPLY_DIFF;
									} // ['ａ', 'ｚ']
								} // ['Ꞌ', 'ｚ']
							} // ['Ꝿ', 'ｚ']
						} // ['Ꜳ', 'ｚ']
					} // ['Ꙁ', 'ｚ']
				} // ['Ⱡ', 'ｚ']
			} // ['ὺ', 'ｚ']
		} // ['σ', 'ｚ']
	}


	// no change
	return ch;

APPLY_DIFF:
	return (char)(ch - diff);

UPPERCASE_IS_EVEN:
	return (char)(ch & ~1u);

UPPERCASE_IS_ODD:
	return (char)((ch - 1) | 1u);
}

char ToLower(char ch)
{
	var diff = 0;
	if (ch >= 65 && ch <= 90) // ['A', 'Z']
	{
		diff = 32;
		goto APPLY_DIFF;
	}

	if (ch >= 192 && ch <= 65338) // ['À', 'Ｚ']
	{
		if (ch <= 975) // ['À', 'Ϗ']
		{
			if (ch <= 434) // ['À', 'Ʋ']
			{
				if (ch <= 400) // ['À', 'Ɛ']
				{
					if (ch <= 382) // ['À', 'ž']
					{
						if (ch <= 311) // ['À', 'ķ']
						{
							if (ch <= 222) // ['À', 'Þ']
							{
								if (ch <= 214) // ['À', 'Ö']
								{
									diff = 32;
									goto APPLY_DIFF;
								} // ['À', 'Ö']
								else if (ch >= 216) // ['Ø', 'Þ']
								{
									diff = 32;
									goto APPLY_DIFF;
								} // ['Ø', 'Þ']
							} // ['À', 'Þ']
							else if (ch >= 256) // ['Ā', 'ķ']
							{
								if (ch <= 303) // ['Ā', 'į']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ā', 'į']
								else if (ch >= 306) // ['Ĳ', 'ķ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ĳ', 'ķ']
							} // ['Ā', 'ķ']
						} // ['À', 'ķ']
						else if (ch >= 313) // ['Ĺ', 'ž']
						{
							if (ch <= 375) // ['Ĺ', 'ŷ']
							{
								if (ch <= 328) // ['Ĺ', 'ň']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ĺ', 'ň']
								else if (ch >= 330) // ['Ŋ', 'ŷ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ŋ', 'ŷ']
							} // ['Ĺ', 'ŷ']
							else if (ch >= 376) // ['Ÿ', 'ž']
							{
								if (ch <= 376) // ['Ÿ', 'Ÿ']
								{
									diff = -121;
									goto APPLY_DIFF;
								} // ['Ÿ', 'Ÿ']

								else if (ch >= 377) // ['Ź', 'ž']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ź', 'ž']
							} // ['Ÿ', 'ž']
						} // ['Ĺ', 'ž']
					} // ['À', 'ž']
					else if (ch >= 385) // ['Ɓ', 'Ɛ']
					{
						if (ch <= 392) // ['Ɓ', 'ƈ']
						{
							if (ch <= 389) // ['Ɓ', 'ƅ']
							{
								if (ch <= 385) // ['Ɓ', 'Ɓ']
								{
									diff = 210;
									goto APPLY_DIFF;
								} // ['Ɓ', 'Ɓ']
								else if (ch >= 386) // ['Ƃ', 'ƅ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ƃ', 'ƅ']
							} // ['Ɓ', 'ƅ']
							else if (ch >= 390) // ['Ɔ', 'ƈ']
							{
								if (ch <= 390) // ['Ɔ', 'Ɔ']
								{
									diff = 206;
									goto APPLY_DIFF;
								} // ['Ɔ', 'Ɔ']
								else if (ch >= 391) // ['Ƈ', 'ƈ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ƈ', 'ƈ']
							} // ['Ɔ', 'ƈ']
						} // ['Ɓ', 'ƈ']
						else if (ch >= 393) // ['Ɖ', 'Ɛ']
						{
							if (ch <= 396) // ['Ɖ', 'ƌ']
							{
								if (ch <= 394) // ['Ɖ', 'Ɗ']
								{
									diff = 205;
									goto APPLY_DIFF;
								} // ['Ɖ', 'Ɗ']
								else if (ch >= 395) // ['Ƌ', 'ƌ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ƌ', 'ƌ']
							} // ['Ɖ', 'ƌ']
							else if (ch >= 398) // ['Ǝ', 'Ɛ']
							{
								if (ch <= 398) // ['Ǝ', 'Ǝ']
								{
									diff = 79;
									goto APPLY_DIFF;
								} // ['Ǝ', 'Ǝ']
								else if (ch >= 399) // ['Ə', 'Ɛ']
								{
									if (ch <= 399) // ['Ə', 'Ə']
									{
										diff = 202;
										goto APPLY_DIFF;
									} // ['Ə', 'Ə']
									else if (ch >= 400) // ['Ɛ', 'Ɛ']
									{
										diff = 203;
										goto APPLY_DIFF;
									} // ['Ɛ', 'Ɛ']
								} // ['Ə', 'Ɛ']
							} // ['Ǝ', 'Ɛ']
						} // ['Ɖ', 'Ɛ']
					} // ['Ɓ', 'Ɛ']
				} // ['À', 'Ɛ']
				else if (ch >= 401) // ['Ƒ', 'Ʋ']
				{
					if (ch <= 413) // ['Ƒ', 'Ɲ']
					{
						if (ch <= 406) // ['Ƒ', 'Ɩ']
						{
							if (ch <= 403) // ['Ƒ', 'Ɠ']
							{
								if (ch <= 402) // ['Ƒ', 'ƒ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ƒ', 'ƒ']
								else if (ch >= 403) // ['Ɠ', 'Ɠ']
								{
									diff = 205;
									goto APPLY_DIFF;
								} // ['Ɠ', 'Ɠ']
							} // ['Ƒ', 'Ɠ']
							else if (ch >= 404) // ['Ɣ', 'Ɩ']
							{
								if (ch <= 404) // ['Ɣ', 'Ɣ']
								{
									diff = 207;
									goto APPLY_DIFF;
								} // ['Ɣ', 'Ɣ']
								else if (ch >= 406) // ['Ɩ', 'Ɩ']
								{
									diff = 211;
									goto APPLY_DIFF;
								} // ['Ɩ', 'Ɩ']
							} // ['Ɣ', 'Ɩ']
						} // ['Ƒ', 'Ɩ']
						else if (ch >= 407) // ['Ɨ', 'Ɲ']
						{
							if (ch <= 409) // ['Ɨ', 'ƙ']
							{
								if (ch <= 407) // ['Ɨ', 'Ɨ']
								{
									diff = 209;
									goto APPLY_DIFF;
								} // ['Ɨ', 'Ɨ']
								else if (ch >= 408) // ['Ƙ', 'ƙ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ƙ', 'ƙ']
							} // ['Ɨ', 'ƙ']
							else if (ch >= 412) // ['Ɯ', 'Ɲ']
							{
								if (ch <= 412) // ['Ɯ', 'Ɯ']
								{
									diff = 211;
									goto APPLY_DIFF;
								} // ['Ɯ', 'Ɯ']
								else if (ch >= 413) // ['Ɲ', 'Ɲ']
								{
									diff = 213;
									goto APPLY_DIFF;
								} // ['Ɲ', 'Ɲ']
							} // ['Ɯ', 'Ɲ']
						} // ['Ɨ', 'Ɲ']
					} // ['Ƒ', 'Ɲ']
					else if (ch >= 415) // ['Ɵ', 'Ʋ']
					{
						if (ch <= 424) // ['Ɵ', 'ƨ']
						{
							if (ch <= 421) // ['Ɵ', 'ƥ']
							{
								if (ch <= 415) // ['Ɵ', 'Ɵ']
								{
									diff = 214;
									goto APPLY_DIFF;
								} // ['Ɵ', 'Ɵ']
								else if (ch >= 416) // ['Ơ', 'ƥ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ơ', 'ƥ']
							} // ['Ɵ', 'ƥ']
							else if (ch >= 422) // ['Ʀ', 'ƨ']
							{
								if (ch <= 422) // ['Ʀ', 'Ʀ']
								{
									diff = 218;
									goto APPLY_DIFF;
								} // ['Ʀ', 'Ʀ']
								else if (ch >= 423) // ['Ƨ', 'ƨ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ƨ', 'ƨ']
							} // ['Ʀ', 'ƨ']
						} // ['Ɵ', 'ƨ']
						else if (ch >= 425) // ['Ʃ', 'Ʋ']
						{
							if (ch <= 429) // ['Ʃ', 'ƭ']
							{
								if (ch <= 425) // ['Ʃ', 'Ʃ']
								{
									diff = 218;
									goto APPLY_DIFF;
								} // ['Ʃ', 'Ʃ']
								else if (ch >= 428) // ['Ƭ', 'ƭ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ƭ', 'ƭ']
							} // ['Ʃ', 'ƭ']
							else if (ch >= 430) // ['Ʈ', 'Ʋ']
							{
								if (ch <= 430) // ['Ʈ', 'Ʈ']
								{
									diff = 218;
									goto APPLY_DIFF;
								} // ['Ʈ', 'Ʈ']
								else if (ch >= 431) // ['Ư', 'Ʋ']
								{
									if (ch <= 432) // ['Ư', 'ư']
									{
										goto UPPERCASE_IS_ODD;
									} // ['Ư', 'ư']
									else if (ch >= 433) // ['Ʊ', 'Ʋ']
									{
										diff = 217;
										goto APPLY_DIFF;
									} // ['Ʊ', 'Ʋ']
								} // ['Ư', 'Ʋ']
							} // ['Ʈ', 'Ʋ']
						} // ['Ʃ', 'Ʋ']
					} // ['Ɵ', 'Ʋ']
				} // ['Ƒ', 'Ʋ']
			} // ['À', 'Ʋ']
			else if (ch >= 435) // ['Ƴ', 'Ϗ']
			{
				if (ch <= 570) // ['Ƴ', 'Ⱥ']
				{
					if (ch <= 476) // ['Ƴ', 'ǜ']
					{
						if (ch <= 445) // ['Ƴ', 'ƽ']
						{
							if (ch <= 439) // ['Ƴ', 'Ʒ']
							{
								if (ch <= 438) // ['Ƴ', 'ƶ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ƴ', 'ƶ']
								else if (ch >= 439) // ['Ʒ', 'Ʒ']
								{
									diff = 219;
									goto APPLY_DIFF;
								} // ['Ʒ', 'Ʒ']
							} // ['Ƴ', 'Ʒ']
							else if (ch >= 440) // ['Ƹ', 'ƽ']
							{
								if (ch <= 441) // ['Ƹ', 'ƹ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ƹ', 'ƹ']
								else if (ch >= 444) // ['Ƽ', 'ƽ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ƽ', 'ƽ']
							} // ['Ƹ', 'ƽ']
						} // ['Ƴ', 'ƽ']
						else if (ch >= 452) // ['Ǆ', 'ǜ']
						{
							if (ch <= 455) // ['Ǆ', 'Ǉ']
							{
								if (ch <= 452) // ['Ǆ', 'Ǆ']
								{
									diff = 2;
									goto APPLY_DIFF;
								} // ['Ǆ', 'Ǆ']
								else if (ch >= 455) // ['Ǉ', 'Ǉ']
								{
									diff = 2;
									goto APPLY_DIFF;
								} // ['Ǉ', 'Ǉ']
							} // ['Ǆ', 'Ǉ']
							else if (ch >= 458) // ['Ǌ', 'ǜ']
							{
								if (ch <= 458) // ['Ǌ', 'Ǌ']
								{
									diff = 2;
									goto APPLY_DIFF;
								} // ['Ǌ', 'Ǌ']
								else if (ch >= 461) // ['Ǎ', 'ǜ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ǎ', 'ǜ']
							} // ['Ǌ', 'ǜ']
						} // ['Ǆ', 'ǜ']
					} // ['Ƴ', 'ǜ']
					else if (ch >= 478) // ['Ǟ', 'Ⱥ']
					{
						if (ch <= 502) // ['Ǟ', 'Ƕ']
						{
							if (ch <= 497) // ['Ǟ', 'Ǳ']
							{
								if (ch <= 495) // ['Ǟ', 'ǯ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ǟ', 'ǯ']
								else if (ch >= 497) // ['Ǳ', 'Ǳ']
								{
									diff = 2;
									goto APPLY_DIFF;
								} // ['Ǳ', 'Ǳ']
							} // ['Ǟ', 'Ǳ']
							else if (ch >= 500) // ['Ǵ', 'Ƕ']
							{
								if (ch <= 501) // ['Ǵ', 'ǵ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ǵ', 'ǵ']
								else if (ch >= 502) // ['Ƕ', 'Ƕ']
								{
									diff = -97;
									goto APPLY_DIFF;
								} // ['Ƕ', 'Ƕ']
							} // ['Ǵ', 'Ƕ']
						} // ['Ǟ', 'Ƕ']
						else if (ch >= 503) // ['Ƿ', 'Ⱥ']
						{
							if (ch <= 543) // ['Ƿ', 'ȟ']
							{
								if (ch <= 503) // ['Ƿ', 'Ƿ']
								{
									diff = -56;
									goto APPLY_DIFF;
								} // ['Ƿ', 'Ƿ']
								else if (ch >= 504) // ['Ǹ', 'ȟ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ǹ', 'ȟ']
							} // ['Ƿ', 'ȟ']
							else if (ch >= 544) // ['Ƞ', 'Ⱥ']
							{
								if (ch <= 544) // ['Ƞ', 'Ƞ']
								{
									diff = -130;
									goto APPLY_DIFF;
								} // ['Ƞ', 'Ƞ']
								else if (ch >= 546) // ['Ȣ', 'Ⱥ']
								{
									if (ch <= 563) // ['Ȣ', 'ȳ']
									{
										goto UPPERCASE_IS_EVEN;
									} // ['Ȣ', 'ȳ']
									else if (ch >= 570) // ['Ⱥ', 'Ⱥ']
									{
										diff = 10795;
										goto APPLY_DIFF;
									} // ['Ⱥ', 'Ⱥ']
								} // ['Ȣ', 'Ⱥ']
							} // ['Ƞ', 'Ⱥ']
						} // ['Ƿ', 'Ⱥ']
					} // ['Ǟ', 'Ⱥ']
				} // ['Ƴ', 'Ⱥ']
				else if (ch >= 571) // ['Ȼ', 'Ϗ']
				{
					if (ch <= 591) // ['Ȼ', 'ɏ']
					{
						if (ch <= 578) // ['Ȼ', 'ɂ']
						{
							if (ch <= 573) // ['Ȼ', 'Ƚ']
							{
								if (ch <= 572) // ['Ȼ', 'ȼ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ȼ', 'ȼ']
								else if (ch >= 573) // ['Ƚ', 'Ƚ']
								{
									diff = -163;
									goto APPLY_DIFF;
								} // ['Ƚ', 'Ƚ']
							} // ['Ȼ', 'Ƚ']
							else if (ch >= 574) // ['Ⱦ', 'ɂ']
							{
								if (ch <= 574) // ['Ⱦ', 'Ⱦ']
								{
									diff = 10792;
									goto APPLY_DIFF;
								} // ['Ⱦ', 'Ⱦ']
								else if (ch >= 577) // ['Ɂ', 'ɂ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ɂ', 'ɂ']
							} // ['Ⱦ', 'ɂ']
						} // ['Ȼ', 'ɂ']
						else if (ch >= 579) // ['Ƀ', 'ɏ']
						{
							if (ch <= 580) // ['Ƀ', 'Ʉ']
							{
								if (ch <= 579) // ['Ƀ', 'Ƀ']
								{
									diff = -195;
									goto APPLY_DIFF;
								} // ['Ƀ', 'Ƀ']
								else if (ch >= 580) // ['Ʉ', 'Ʉ']
								{
									diff = 69;
									goto APPLY_DIFF;
								} // ['Ʉ', 'Ʉ']
							} // ['Ƀ', 'Ʉ']
							else if (ch >= 581) // ['Ʌ', 'ɏ']
							{
								if (ch <= 581) // ['Ʌ', 'Ʌ']
								{
									diff = 71;
									goto APPLY_DIFF;
								} // ['Ʌ', 'Ʌ']
								else if (ch >= 582) // ['Ɇ', 'ɏ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ɇ', 'ɏ']
							} // ['Ʌ', 'ɏ']
						} // ['Ƀ', 'ɏ']
					} // ['Ȼ', 'ɏ']
					else if (ch >= 880) // ['Ͱ', 'Ϗ']
					{
						if (ch <= 906) // ['Ͱ', 'Ί']
						{
							if (ch <= 887) // ['Ͱ', 'ͷ']
							{
								if (ch <= 883) // ['Ͱ', 'ͳ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ͱ', 'ͳ']
								else if (ch >= 886) // ['Ͷ', 'ͷ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ͷ', 'ͷ']
							} // ['Ͱ', 'ͷ']
							else if (ch >= 902) // ['Ά', 'Ί']
							{
								if (ch <= 902) // ['Ά', 'Ά']
								{
									diff = 38;
									goto APPLY_DIFF;
								} // ['Ά', 'Ά']
								else if (ch >= 904) // ['Έ', 'Ί']
								{
									diff = 37;
									goto APPLY_DIFF;
								} // ['Έ', 'Ί']
							} // ['Ά', 'Ί']
						} // ['Ͱ', 'Ί']
						else if (ch >= 908) // ['Ό', 'Ϗ']
						{
							if (ch <= 911) // ['Ό', 'Ώ']
							{
								if (ch <= 908) // ['Ό', 'Ό']
								{
									diff = 64;
									goto APPLY_DIFF;
								} // ['Ό', 'Ό']
								else if (ch >= 910) // ['Ύ', 'Ώ']
								{
									diff = 63;
									goto APPLY_DIFF;
								} // ['Ύ', 'Ώ']
							} // ['Ό', 'Ώ']
							else if (ch >= 913) // ['Α', 'Ϗ']
							{
								if (ch <= 929) // ['Α', 'Ρ']
								{
									diff = 32;
									goto APPLY_DIFF;
								} // ['Α', 'Ρ']
								else if (ch >= 931) // ['Σ', 'Ϗ']
								{
									if (ch <= 939) // ['Σ', 'Ϋ']
									{
										diff = 32;
										goto APPLY_DIFF;
									} // ['Σ', 'Ϋ']
									else if (ch >= 975) // ['Ϗ', 'Ϗ']
									{
										diff = 8;
										goto APPLY_DIFF;
									} // ['Ϗ', 'Ϗ']
								} // ['Σ', 'Ϗ']
							} // ['Α', 'Ϗ']
						} // ['Ό', 'Ϗ']
					} // ['Ͱ', 'Ϗ']
				} // ['Ȼ', 'Ϗ']
			} // ['Ƴ', 'Ϗ']
		} // ['À', 'Ϗ']
		else if (ch >= 984) // ['Ϙ', 'Ｚ']
		{
			if (ch <= 8140) // ['Ϙ', 'ῌ']
			{
				if (ch <= 7951) // ['Ϙ', 'Ἇ']
				{
					if (ch <= 1153) // ['Ϙ', 'ҁ']
					{
						if (ch <= 1019) // ['Ϙ', 'ϻ']
						{
							if (ch <= 1016) // ['Ϙ', 'ϸ']
							{
								if (ch <= 1007) // ['Ϙ', 'ϯ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ϙ', 'ϯ']
								else if (ch >= 1015) // ['Ϸ', 'ϸ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ϸ', 'ϸ']
							} // ['Ϙ', 'ϸ']
							else if (ch >= 1017) // ['Ϲ', 'ϻ']
							{
								if (ch <= 1017) // ['Ϲ', 'Ϲ']
								{
									diff = -7;
									goto APPLY_DIFF;
								} // ['Ϲ', 'Ϲ']
								else if (ch >= 1018) // ['Ϻ', 'ϻ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ϻ', 'ϻ']
							} // ['Ϲ', 'ϻ']
						} // ['Ϙ', 'ϻ']
						else if (ch >= 1021) // ['Ͻ', 'ҁ']
						{
							if (ch <= 1039) // ['Ͻ', 'Џ']
							{
								if (ch <= 1023) // ['Ͻ', 'Ͽ']
								{
									diff = -130;
									goto APPLY_DIFF;
								} // ['Ͻ', 'Ͽ']
								else if (ch >= 1024) // ['Ѐ', 'Џ']
								{
									diff = 80;
									goto APPLY_DIFF;
								} // ['Ѐ', 'Џ']
							} // ['Ͻ', 'Џ']
							else if (ch >= 1040) // ['А', 'ҁ']
							{
								if (ch <= 1071) // ['А', 'Я']
								{
									diff = 32;
									goto APPLY_DIFF;
								} // ['А', 'Я']
								else if (ch >= 1120) // ['Ѡ', 'ҁ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ѡ', 'ҁ']
							} // ['А', 'ҁ']
						} // ['Ͻ', 'ҁ']
					} // ['Ϙ', 'ҁ']
					else if (ch >= 1162) // ['Ҋ', 'Ἇ']
					{
						if (ch <= 1315) // ['Ҋ', 'ԣ']
						{
							if (ch <= 1216) // ['Ҋ', 'Ӏ']
							{
								if (ch <= 1215) // ['Ҋ', 'ҿ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ҋ', 'ҿ']
								else if (ch >= 1216) // ['Ӏ', 'Ӏ']
								{
									diff = 15;
									goto APPLY_DIFF;
								} // ['Ӏ', 'Ӏ']
							} // ['Ҋ', 'Ӏ']
							else if (ch >= 1217) // ['Ӂ', 'ԣ']
							{
								if (ch <= 1230) // ['Ӂ', 'ӎ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ӂ', 'ӎ']
								else if (ch >= 1232) // ['Ӑ', 'ԣ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ӑ', 'ԣ']
							} // ['Ӂ', 'ԣ']
						} // ['Ҋ', 'ԣ']
						else if (ch >= 1329) // ['Ա', 'Ἇ']
						{
							if (ch <= 4293) // ['Ա', 'Ⴥ']
							{
								if (ch <= 1366) // ['Ա', 'Ֆ']
								{
									diff = 48;
									goto APPLY_DIFF;
								} // ['Ա', 'Ֆ']
								else if (ch >= 4256) // ['Ⴀ', 'Ⴥ']
								{
									diff = 7264;
									goto APPLY_DIFF;
								} // ['Ⴀ', 'Ⴥ']
							} // ['Ա', 'Ⴥ']
							else if (ch >= 7680) // ['Ḁ', 'Ἇ']
							{
								if (ch <= 7829) // ['Ḁ', 'ẕ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ḁ', 'ẕ']
								else if (ch >= 7840) // ['Ạ', 'Ἇ']
								{
									if (ch <= 7935) // ['Ạ', 'ỿ']
									{
										goto UPPERCASE_IS_EVEN;
									} // ['Ạ', 'ỿ']
									else if (ch >= 7944) // ['Ἀ', 'Ἇ']
									{
										diff = -8;
										goto APPLY_DIFF;
									} // ['Ἀ', 'Ἇ']
								} // ['Ạ', 'Ἇ']
							} // ['Ḁ', 'Ἇ']
						} // ['Ա', 'Ἇ']
					} // ['Ҋ', 'Ἇ']
				} // ['Ϙ', 'Ἇ']
				else if (ch >= 7960) // ['Ἐ', 'ῌ']
				{
					if (ch <= 8031) // ['Ἐ', 'Ὗ']
					{
						if (ch <= 8013) // ['Ἐ', 'Ὅ']
						{
							if (ch <= 7983) // ['Ἐ', 'Ἧ']
							{
								if (ch <= 7965) // ['Ἐ', 'Ἕ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ἐ', 'Ἕ']
								else if (ch >= 7976) // ['Ἠ', 'Ἧ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ἠ', 'Ἧ']
							} // ['Ἐ', 'Ἧ']
							else if (ch >= 7992) // ['Ἰ', 'Ὅ']
							{
								if (ch <= 7999) // ['Ἰ', 'Ἷ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ἰ', 'Ἷ']
								else if (ch >= 8008) // ['Ὀ', 'Ὅ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ὀ', 'Ὅ']
							} // ['Ἰ', 'Ὅ']
						} // ['Ἐ', 'Ὅ']
						else if (ch >= 8025) // ['Ὑ', 'Ὗ']
						{
							if (ch <= 8027) // ['Ὑ', 'Ὓ']
							{
								if (ch <= 8025) // ['Ὑ', 'Ὑ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ὑ', 'Ὑ']
								else if (ch >= 8027) // ['Ὓ', 'Ὓ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ὓ', 'Ὓ']
							} // ['Ὑ', 'Ὓ']
							else if (ch >= 8029) // ['Ὕ', 'Ὗ']
							{
								if (ch <= 8029) // ['Ὕ', 'Ὕ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ὕ', 'Ὕ']
								else if (ch >= 8031) // ['Ὗ', 'Ὗ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ὗ', 'Ὗ']
							} // ['Ὕ', 'Ὗ']
						} // ['Ὑ', 'Ὗ']
					} // ['Ἐ', 'Ὗ']
					else if (ch >= 8040) // ['Ὠ', 'ῌ']
					{
						if (ch <= 8111) // ['Ὠ', 'ᾯ']
						{
							if (ch <= 8079) // ['Ὠ', 'ᾏ']
							{
								if (ch <= 8047) // ['Ὠ', 'Ὧ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ὠ', 'Ὧ']
								else if (ch >= 8072) // ['ᾈ', 'ᾏ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['ᾈ', 'ᾏ']
							} // ['Ὠ', 'ᾏ']
							else if (ch >= 8088) // ['ᾘ', 'ᾯ']
							{
								if (ch <= 8095) // ['ᾘ', 'ᾟ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['ᾘ', 'ᾟ']
								else if (ch >= 8104) // ['ᾨ', 'ᾯ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['ᾨ', 'ᾯ']
							} // ['ᾘ', 'ᾯ']
						} // ['Ὠ', 'ᾯ']
						else if (ch >= 8120) // ['Ᾰ', 'ῌ']
						{
							if (ch <= 8123) // ['Ᾰ', 'Ά']
							{
								if (ch <= 8121) // ['Ᾰ', 'Ᾱ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ᾰ', 'Ᾱ']
								else if (ch >= 8122) // ['Ὰ', 'Ά']
								{
									diff = -74;
									goto APPLY_DIFF;
								} // ['Ὰ', 'Ά']
							} // ['Ᾰ', 'Ά']
							else if (ch >= 8124) // ['ᾼ', 'ῌ']
							{
								if (ch <= 8124) // ['ᾼ', 'ᾼ']
								{
									diff = -9;
									goto APPLY_DIFF;
								} // ['ᾼ', 'ᾼ']
								else if (ch >= 8136) // ['Ὲ', 'ῌ']
								{
									if (ch <= 8139) // ['Ὲ', 'Ή']
									{
										diff = -86;
										goto APPLY_DIFF;
									} // ['Ὲ', 'Ή']
									else if (ch >= 8140) // ['ῌ', 'ῌ']
									{
										diff = -9;
										goto APPLY_DIFF;
									} // ['ῌ', 'ῌ']
								} // ['Ὲ', 'ῌ']
							} // ['ᾼ', 'ῌ']
						} // ['Ᾰ', 'ῌ']
					} // ['Ὠ', 'ῌ']
				} // ['Ἐ', 'ῌ']
			} // ['Ϙ', 'ῌ']
			else if (ch >= 8152) // ['Ῐ', 'Ｚ']
			{
				if (ch <= 11364) // ['Ῐ', 'Ɽ']
				{
					if (ch <= 8188) // ['Ῐ', 'ῼ']
					{
						if (ch <= 8171) // ['Ῐ', 'Ύ']
						{
							if (ch <= 8155) // ['Ῐ', 'Ί']
							{

								if (ch <= 8153) // ['Ῐ', 'Ῑ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ῐ', 'Ῑ']
								else if (ch >= 8154) // ['Ὶ', 'Ί']
								{
									diff = -100;
									goto APPLY_DIFF;
								} // ['Ὶ', 'Ί']
							} // ['Ῐ', 'Ί']
							else if (ch >= 8168) // ['Ῠ', 'Ύ']
							{
								if (ch <= 8169) // ['Ῠ', 'Ῡ']
								{
									diff = -8;
									goto APPLY_DIFF;
								} // ['Ῠ', 'Ῡ']
								else if (ch >= 8170) // ['Ὺ', 'Ύ']
								{
									diff = -112;
									goto APPLY_DIFF;
								} // ['Ὺ', 'Ύ']
							} // ['Ῠ', 'Ύ']
						} // ['Ῐ', 'Ύ']
						else if (ch >= 8172) // ['Ῥ', 'ῼ']
						{
							if (ch <= 8185) // ['Ῥ', 'Ό']
							{
								if (ch <= 8172) // ['Ῥ', 'Ῥ']
								{
									diff = -7;
									goto APPLY_DIFF;
								} // ['Ῥ', 'Ῥ']
								else if (ch >= 8184) // ['Ὸ', 'Ό']
								{
									diff = -128;
									goto APPLY_DIFF;
								} // ['Ὸ', 'Ό']
							} // ['Ῥ', 'Ό']
							else if (ch >= 8186) // ['Ὼ', 'ῼ']
							{
								if (ch <= 8187) // ['Ὼ', 'Ώ']
								{
									diff = -126;
									goto APPLY_DIFF;
								} // ['Ὼ', 'Ώ']
								else if (ch >= 8188) // ['ῼ', 'ῼ']
								{
									diff = -9;
									goto APPLY_DIFF;
								} // ['ῼ', 'ῼ']
							} // ['Ὼ', 'ῼ']
						} // ['Ῥ', 'ῼ']
					} // ['Ῐ', 'ῼ']
					else if (ch >= 8498) // ['Ⅎ', 'Ɽ']
					{
						if (ch <= 9423) // ['Ⅎ', 'Ⓩ']
						{
							if (ch <= 8559) // ['Ⅎ', 'Ⅿ']
							{
								if (ch <= 8498) // ['Ⅎ', 'Ⅎ']
								{
									diff = 28;
									goto APPLY_DIFF;
								} // ['Ⅎ', 'Ⅎ']
								else if (ch >= 8544) // ['Ⅰ', 'Ⅿ']
								{
									diff = 16;
									goto APPLY_DIFF;
								} // ['Ⅰ', 'Ⅿ']
							} // ['Ⅎ', 'Ⅿ']
							else if (ch >= 8579) // ['Ↄ', 'Ⓩ']
							{
								if (ch <= 8580) // ['Ↄ', 'ↄ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ↄ', 'ↄ']
								else if (ch >= 9398) // ['Ⓐ', 'Ⓩ']
								{
									diff = 26;
									goto APPLY_DIFF;
								} // ['Ⓐ', 'Ⓩ']
							} // ['Ↄ', 'Ⓩ']
						} // ['Ⅎ', 'Ⓩ']
						else if (ch >= 11264) // ['Ⰰ', 'Ɽ']
						{
							if (ch <= 11361) // ['Ⰰ', 'ⱡ']
							{
								if (ch <= 11310) // ['Ⰰ', 'Ⱞ']
								{
									diff = 48;
									goto APPLY_DIFF;
								} // ['Ⰰ', 'Ⱞ']
								else if (ch >= 11360) // ['Ⱡ', 'ⱡ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ⱡ', 'ⱡ']
							} // ['Ⰰ', 'ⱡ']
							else if (ch >= 11362) // ['Ɫ', 'Ɽ']
							{
								if (ch <= 11362) // ['Ɫ', 'Ɫ']
								{
									diff = -10743;
									goto APPLY_DIFF;
								} // ['Ɫ', 'Ɫ']
								else if (ch >= 11363) // ['Ᵽ', 'Ɽ']
								{
									if (ch <= 11363) // ['Ᵽ', 'Ᵽ']
									{
										diff = -3814;
										goto APPLY_DIFF;
									} // ['Ᵽ', 'Ᵽ']
									else if (ch >= 11364) // ['Ɽ', 'Ɽ']
									{
										diff = -10727;
										goto APPLY_DIFF;
									} // ['Ɽ', 'Ɽ']
								} // ['Ᵽ', 'Ɽ']
							} // ['Ɫ', 'Ɽ']
						} // ['Ⰰ', 'Ɽ']
					} // ['Ⅎ', 'Ɽ']
				} // ['Ῐ', 'Ɽ']
				else if (ch >= 11367) // ['Ⱨ', 'Ｚ']
				{
					if (ch <= 42591) // ['Ⱨ', 'ꙟ']
					{
						if (ch <= 11375) // ['Ⱨ', 'Ɐ']
						{
							if (ch <= 11373) // ['Ⱨ', 'Ɑ']
							{
								if (ch <= 11372) // ['Ⱨ', 'ⱬ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ⱨ', 'ⱬ']
								else if (ch >= 11373) // ['Ɑ', 'Ɑ']
								{
									diff = -10780;
									goto APPLY_DIFF;
								} // ['Ɑ', 'Ɑ']
							} // ['Ⱨ', 'Ɑ']
							else if (ch >= 11374) // ['Ɱ', 'Ɐ']
							{
								if (ch <= 11374) // ['Ɱ', 'Ɱ']
								{
									diff = -10749;
									goto APPLY_DIFF;
								} // ['Ɱ', 'Ɱ']
								else if (ch >= 11375) // ['Ɐ', 'Ɐ']
								{
									diff = -10783;
									goto APPLY_DIFF;
								} // ['Ɐ', 'Ɐ']
							} // ['Ɱ', 'Ɐ']
						} // ['Ⱨ', 'Ɐ']
						else if (ch >= 11378) // ['Ⱳ', 'ꙟ']
						{
							if (ch <= 11382) // ['Ⱳ', 'ⱶ']
							{
								if (ch <= 11379) // ['Ⱳ', 'ⱳ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ⱳ', 'ⱳ']
								else if (ch >= 11381) // ['Ⱶ', 'ⱶ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ⱶ', 'ⱶ']
							} // ['Ⱳ', 'ⱶ']
							else if (ch >= 11392) // ['Ⲁ', 'ꙟ']
							{
								if (ch <= 11491) // ['Ⲁ', 'ⳣ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ⲁ', 'ⳣ']
								else if (ch >= 42560) // ['Ꙁ', 'ꙟ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꙁ', 'ꙟ']
							} // ['Ⲁ', 'ꙟ']
						} // ['Ⱳ', 'ꙟ']
					} // ['Ⱨ', 'ꙟ']
					else if (ch >= 42594) // ['Ꙣ', 'Ｚ']
					{
						if (ch <= 42863) // ['Ꙣ', 'ꝯ']
						{
							if (ch <= 42647) // ['Ꙣ', 'ꚗ']
							{
								if (ch <= 42605) // ['Ꙣ', 'ꙭ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꙣ', 'ꙭ']
								else if (ch >= 42624) // ['Ꚁ', 'ꚗ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꚁ', 'ꚗ']
							} // ['Ꙣ', 'ꚗ']
							else if (ch >= 42786) // ['Ꜣ', 'ꝯ']
							{
								if (ch <= 42799) // ['Ꜣ', 'ꜯ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꜣ', 'ꜯ']
								else if (ch >= 42802) // ['Ꜳ', 'ꝯ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꜳ', 'ꝯ']
							} // ['Ꜣ', 'ꝯ']
						} // ['Ꙣ', 'ꝯ']
						else if (ch >= 42873) // ['Ꝺ', 'Ｚ']
						{
							if (ch <= 42877) // ['Ꝺ', 'Ᵹ']
							{
								if (ch <= 42876) // ['Ꝺ', 'ꝼ']
								{
									goto UPPERCASE_IS_ODD;
								} // ['Ꝺ', 'ꝼ']
								else if (ch >= 42877) // ['Ᵹ', 'Ᵹ']
								{
									diff = -35332;
									goto APPLY_DIFF;
								} // ['Ᵹ', 'Ᵹ']
							} // ['Ꝺ', 'Ᵹ']
							else if (ch >= 42878) // ['Ꝿ', 'Ｚ']
							{
								if (ch <= 42887) // ['Ꝿ', 'ꞇ']
								{
									goto UPPERCASE_IS_EVEN;
								} // ['Ꝿ', 'ꞇ']
								else if (ch >= 42891) // ['Ꞌ', 'Ｚ']
								{
									if (ch <= 42892) // ['Ꞌ', 'ꞌ']
									{
										goto UPPERCASE_IS_ODD;
									} // ['Ꞌ', 'ꞌ']
									else if (ch >= 65313) // ['Ａ', 'Ｚ']
									{
										diff = 32;
										goto APPLY_DIFF;
									} // ['Ａ', 'Ｚ']
								} // ['Ꞌ', 'Ｚ']
							} // ['Ꝿ', 'Ｚ']
						} // ['Ꝺ', 'Ｚ']
					} // ['Ꙣ', 'Ｚ']
				} // ['Ⱨ', 'Ｚ']
			} // ['Ῐ', 'Ｚ']
		} // ['Ϙ', 'Ｚ']
	}


	// no change
	return ch;

APPLY_DIFF:
	return (char)(ch + diff);

UPPERCASE_IS_EVEN:
	return (char)(ch | 1u);

UPPERCASE_IS_ODD:
	return (char)((ch + 1) & ~1u);
}

