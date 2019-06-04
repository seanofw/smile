//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Command-Line Runner)
//  Copyright 2004-2019 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include "stdafx.h"
#include <stdlib.h>

#include "style.h"

static Bool _firstTime = True;
static UInt32 _rand = 0;

static String GenerateMazeOfTwistyPassages(void)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	_rand = _rand * 69069 + 1;
	switch ((_rand >> 16) % 4) {
		case 0:
			StringBuilder_AppendFormat(stringBuilder, "You are in ");
			break;
		case 1:
			StringBuilder_AppendFormat(stringBuilder, "You see ");
			break;
		case 2:
			StringBuilder_AppendFormat(stringBuilder, "You are lost in ");
			break;
		case 3:
			StringBuilder_AppendFormat(stringBuilder, "You have found ");
			break;
	}

	_rand = _rand * 69069 + 1;
	switch ((_rand >> 16) % 6) {
		case 0:
			StringBuilder_AppendFormat(stringBuilder, "\033[0;1ma little maze of twisty passages\033[0m");
			break;
		case 1:
			StringBuilder_AppendFormat(stringBuilder, "\033[0;1ma twisty maze of little passages\033[0m");
			break;
		case 2:
			StringBuilder_AppendFormat(stringBuilder, "\033[0;1ma little twisty maze of passages\033[0m");
			break;
		case 3:
			StringBuilder_AppendFormat(stringBuilder, "\033[0;1ma twisty little maze of passages\033[0m");
			break;
		case 4:
			StringBuilder_AppendFormat(stringBuilder, "\033[0;1ma maze of twisty little passages\033[0m");
			break;
		case 5:
			StringBuilder_AppendFormat(stringBuilder, "\033[0;1ma maze of little twisty passages\033[0m");
			break;
	}

	_rand = _rand * 69069 + 1;
	switch ((_rand >> 16) % 3) {
		case 0:
			StringBuilder_AppendFormat(stringBuilder, ", all alike.");
			break;
		case 1:
			StringBuilder_AppendFormat(stringBuilder, ", all the same.");
			break;
		case 2:
			StringBuilder_AppendFormat(stringBuilder, ", all different.");
			break;
	}

	return StringBuilder_ToString(stringBuilder);
}

static String GenerateJokeResponse(void)
{
	if (_firstTime) {
		Int64 entropy = GetBaselineEntropy();
		_rand = (UInt32)entropy * 69069 + 1;
		_firstTime = False;
	}

	_rand = _rand * 69069 + 1;

	switch ((_rand >> 16) % 8) {
		case 0:
			{
				static const char *putDowns[] = {
					"You're doing well.  Good for you.",
					"No, I think you're doing just fine.",
					"You can't possibly be this lost.",
					"Command lines really aren't that complicated.",
					"We put a detailed user's manual under your chair, just for you.\n"
						"You should bend over and check it out.",
					"\033[0;1mStep 1.\033[0m Unplug the computer.  \033[0;1mStep 2.\033[0m Leave it unplugged.",
				};
				_rand = _rand * 69069 + 1;
				return String_FromC(putDowns[(_rand >> 16) % (sizeof(putDowns) / sizeof(char *))]);
		}
		case 1:
			{
				static const char *funnyAnswers[] = {
					"\033[0;1;33mDon't panic!\033[0m",
					"A Saint Bernard with a small barrel of brandy will be along shortly.",
					"Do you need somebody?  And not just anybody?",
					"Start bailing out the boat, and I'll throw you a life jacket!",
					"Have you tried turning it off and back on again?",
					"This is the helpdesk.  Please hold.",
					"This is Judy from \033[0;1;36mTime Life Magazines\033[0m, how can I help you?",
					"Your call is important to us.  Please stay on the line.",
					"We are experiencing abnormal call volume at this time.",
					"You see a lamp and a grue here.  Try not to be eaten by the grue.",
					"Press the 'Reset' button for three seconds while grounding yourself on something metal.",
					"Try hitting it with a rock.",
					"Smile is an adjunctive encabulator, with dynamic spurving and a generalized differential grammeter.",
					"Have you tried travelling back in time to become your own grandfather?",
					"Try moving your knight to king's bishop 6.",
					"Go get a golf cart motor and a thousand-volt capacimator.",
					"Can you believe it's not butter?",
					"Still not as bad as \"Superman IV: The Quest for Peace.\"",
					"man man",
					"You just lost The Game.",	// ...also true if you're reading the source code.
				};
				_rand = _rand * 69069 + 1;
				return String_FromC(funnyAnswers[(_rand >> 16) % (sizeof(funnyAnswers) / sizeof(char *))]);
			}
		case 2:
			{
				static const char *actualHelp[] = {
					"If things are that bad, have you considered dialing \033[0;1;36m911\033[0m?",
					"Have you considered asking the fine people at \033[0;1;36mStackOverflow\033[0m?",
					"There's lots of documentation on \033[0;1;36msmile-lang.org\033[0m.",
				};
				_rand = _rand * 69069 + 1;
				return String_FromC(actualHelp[(_rand >> 16) % (sizeof(actualHelp) / sizeof(char *))]);
			}
		case 3:
			{
				static const char *emergencyVehicles[] = {
					"helicopter", "military plane", "police car", "SWAT truck", "pickup truck", "cargo boat",
					"semi truck", "jetliner", "steamship", "submarine", "van", "truck",
				};
				static const char *emergencyResponders[] = {
					"Marines", "Army men", "Navy men", "Coast Guard men", "firefighters", "police",
					"volunteer firefighters", "security guards", "mall cops", "Boy Scouts", "Girl Scouts",
					"people who slept at a Holiday Inn Express last night",
				};
				Int rand = _rand = _rand * 69069 + 1;
				Int rand2 = _rand = _rand * 69069 + 1;
				return String_Format("Hang on, we're sending in a %s full of %s!",
					emergencyVehicles[(rand >> 16) % (sizeof(emergencyVehicles) / sizeof(char *))],
					emergencyResponders[(rand2 >> 16) % (sizeof(emergencyResponders) / sizeof(char *))]);
			}
		case 4:
			{
				static const char *searchEngines[] = {
					"A \033[0;1;36mGoogle\033[0m", "A \033[0;1;36mBing\033[0m", "A \033[0;1;36mYahoo\033[0m",
					"A \033[0;1;36mBaidu\033[0m", "A \033[0;1;36mDuck Duck Go\033[0m", "A \033[0;1;36mWolfram Alpha\033[0m",
					"An \033[0;1;36mAltaVista\033[0m", "A \033[0;1;36mWikipedia\033[0m", "An \033[0;1;36marchive.org\033[0m",
				};
				_rand = _rand * 69069 + 1;
				return String_Format("%s search can answer a lot of questions.",
					searchEngines[(_rand >> 16) % (sizeof(searchEngines) / sizeof(char *))]);
			}
		case 5:
			{
				static const char *socialMediaSites[] = {
					"Twitter", "Facebook", "Google+", "MySpace", "Instagram", "Snapchat",
					"Pinterest", "YouTube", "LinkedIn", "Flickr", "Reddit", "WhatsApp",
				};
				_rand = _rand * 69069 + 1;
				return String_Format("Try asking your friends on \033[0;1;36m%s\033[0m.",
					socialMediaSites[(_rand >> 16) % (sizeof(socialMediaSites) / sizeof(char *))]);
			}
		case 6:
			{
				static const char *hobbies[] = {
					"basket-weaving", "knitting", "crochet", "pottery", "flower arrangement", "stamp collecting",
					"coin collecting", "whittling", "needlework", "gardening", "brass rubbing",
				};
				_rand = _rand * 69069 + 1;
				return String_Format("I hear %s is a much easier hobby than programming.",
					hobbies[(_rand >> 16) % (sizeof(hobbies) / sizeof(char *))]);
			}
		default:
		case 7:
			return GenerateMazeOfTwistyPassages();
	}
}

void ShowHelp(String commandLine)
{
	String input = String_Trim(commandLine);

	if (String_IsNullOrEmpty(input)) {
		printf_styled(
			"\033[0m\n"
			"This is the Smile REPL (read-evaluate-print loop).  At the\n"
			"REPL, you can type ordinary Smile code to run it, and you can\n"
			"type special commands to perform common operations for\n"
			"manipulating, running, testing, and debugging Smile programs.\n"
			"\n"
			"\033[1mCommands:\033[0m\n"
			"\033[1;36m  cd        cls       exit  location  run\033[0m\n"
			"\033[1;36m  clear     continue  go    ls\033[0m\n"
			"\033[1;36m  closure   dir       help  pwd\033[0m\n"
			"\033[1;36m  closures  eval      loc   quit\033[0m\n"
			"\n"
			"For help on any specific command, type \"\033[1;36mhelp \033[0;36m[command]\033[0m\".\n"
			"\n"
			"To exit the Smile REPL, type \"\033[1;36mexit\033[0m\" and press Enter.\n"
			"\n"
		);
	}
	else if (String_EqualsC(input, "cd")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33mcd \033[0;36m[path]\033[0m\n"
			"\n"
			"Change the current working directory to the given path (which\n"
			"may be either absolute or relative, just like with the shell's cd\n"
			"command).\n"
			"\n"
			"If no arguments are provided, this displays the current path.\n"
			"\n"
		);
	}
	else if (String_EqualsC(input, "clear") || String_EqualsC(input, "cls")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33m%s\033[0m\n"
			"\n"
			"Clear the screen.\n"
			"\n",
			String_ToC(input)
		);
	}
	else if (String_EqualsC(input, "closure")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33mclosure \033[0;36m[number]\033[0m\n"
			"\n"
			"Show the state of the current (deepest) closure.  This will list\n"
			"the variables defined within the given closure, along with their\n"
			"typeof symbols and their values (if they are intrinsic types like\n"
			"Integer32 or String).  (If their values are very long, they will\n"
			"be truncated.)  If a closure number is provided, that closure's\n"
			"variable list will be displayed instead of the zeroth (deepest)\n"
			"closure.\n"
			"\n"
		);
	}
	else if (String_EqualsC(input, "closures")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33mclosures\033[0m\n"
			"\n"
			"Show the state of all of the current closures, all the way back to\n"
			"the root.  This is a shorthand for simply running \"closure 0\",\n"
			"\"closure 1\", \"closure 2\", and so on until no closures are left.\n"
			"\n"
		);
	}
	else if (String_EqualsC(input, "continue") || String_EqualsC(input, "go")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33m%s\033[0m\n"
			"\n"
			"Continue executing the program from the current location.\n"
			"\n",
			String_ToC(input)
		);
	}
	else if (String_EqualsC(input, "dir") || String_EqualsC(input, "ls")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33m%s \033[0;36m[options] [path]\033[0m\n"
			"\n"
			"Display a list of files in the current directory, or in the given\n"
			"directory if a path is provided.\n"
			"\n"
			"This implementation of dir/ls is somewhat primitive, but it includes\n"
			"enough options to be useful:\n"
			"\n"
			"  \033[0;1;36m-l   \033[0mLong mode ('dir' uses this automatically).\n"
			"  \033[0;1;36m-a   \033[0mShow all files, including dotfiles and hidden files.\n"
			"  \033[0;1;36m-f   \033[0mPlain mode:  Don't include type suffixes like '/' and '*'.\n"
			"  \033[0;1;36m-F   \033[0mFull mode:  Include type suffixes like '/' and '*'.\n"
			"\n",
			String_ToC(input)
		);
	}
	else if (String_EqualsC(input, "eval")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33meval \033[0;36mexpression\033[0m\n"
			"\n"
			"Perform the given expression within the current closure, and display the\n"
			"result of evaluating it.  You can also evaluate most expressions by typing\n"
			"them directly, but eval lets you use names like 'cd' and 'ls' and even\n"
			"'eval' itself that might otherwise be reserved by the REPL.\n"
			"\n"
			"Note that this command has no restrictions, and it can do anything the main\n"
			"program can do:  The expression and can modify and declare variables in the\n"
			"current closure, can define #syntax, can alter properties, can call functions\n"
			"and methods, loop, recurse, and generally perform any operation the main\n"
			"program can perform.\n"
			"\n"
			"By default, eval prints out the result of evaluating the expression.  If you\n"
			"would like to silence eval's normal output, place a '@' character before the\n"
			"expression (this can be done with directly-typed expressions as well).  (Note:\n"
			"The '@' prefix is not part of the Smile programming language, but is instead a\n"
			"useful hack provided by the REPL.)\n"
			"\n"
		);
	}
	else if (String_EqualsC(input, "exit") || String_EqualsC(input, "quit")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33m%s\033[0m\n"
			"\n"
			"Stop the current program and exit the REPL.\n"
			"\n",
			String_ToC(input)
		);
	}
	else if (String_EqualsC(input, "help")) {
		printf_styled(
			"\n"
			"%s\n"
			"\n",
			String_ToC(GenerateJokeResponse())
		);
	}
	else if (String_EqualsC(input, "loc") || String_EqualsC(input, "location")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33m%s\033[0m\n"
			"\n"
			"Show the source location where the current program is stopped,\n"
			"i.e., where the continue/go commands will resume execution.\n"
			"\n",
			String_ToC(input)
		);
	}
	else if (String_EqualsC(input, "pwd")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33mpwd\033[0m\n"
			"\n"
			"Like the shell command of the same name, this prints the current\n"
			"directory path.  The path is always shown with components separated\n"
			"by forward slashes (/), even on systems that use other separator\n"
			"characters.\n"
			"\n"
		);
	}
	else if (String_EqualsC(input, "run")) {
		printf_styled(
			"\033[0;1;37mUsage: \033[1;33mrun \033[0;36mfile.sm\033[0m\n"
			"\n"
			"Load, parse, and evaluate the code in the given source file.\n"
			"\n"
		);
	}
	else {
		printf_styled("\033[0;1;31mNo help available.\033[0m\n\n");
	}
}
