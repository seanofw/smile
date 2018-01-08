# Contributing to Smile

Thank you for being willing to contribute!  Smile is a small project, and we need all the help we can get!

Below is a set of guidelines for how to contribute.



## Code of Conduct

First, make sure you read the [Community Standards](COMMUNITY-STANDARDS.md).  Please make an effort to be nice and to play nice.  By participating, you are expected to uphold these standards.  Please report unacceptable behavior directly to me, <sean AT werkema D0T com>.



## I have a question!

Please don't file a bug report or feature request just to ask a question.  If you're lost or confused or not sure how to do something, please consider asking on [StackOverflow](http://www.stackoverflow.com) instead.

Also, there's lots of documentation on [smile-lang.org](http://www.smile-lang.org).  Your question might be answered already there, so please be sure to read that first.

I don't have any kind of chat thingy set up, so there's not yet a live-chat system for Smile programmers (that I know of).



## How can I contribute?

### Reporting Bugs

> "Works on my machine."
>
> -- Every programmer ever

Programming languages are big, complex things, and it's possible that there will be bugs in the Smile runtime.  There are unit tests to help prevent bugs, but there's always the possibility of a corner case we've missed.

So if you think you've found a bug, please first follow these steps:

1. **First, make sure you can reproduce it**, reliably and consistently.  If you can't make the bug happen predictably, there's no way we'll be able to.
2. **Second, boil it down to a simple test case.**  Don't submit a bug report that's a 10,000-line program.  Ideally, you should be able to reproduce the bug in a single line of code, but even if it requires multiple lines of code, there's no reason for a bug report to ever have more than ten.
3. **Third, make sure it hasn't already been reported!**  Use due diligence, and check the [Issues](https://github.com/seanofw/smile/issues) tab on GitHub to make sure somebody else hasn't already talked about it.  Also, check [StackOverflow](http://www.stackoverflow.com) and see if it's been mentioned there.  Please don't report a bug that's already been reported, and please don't add "me-too" comments to a bug report.

Okay, so you've found a new bug.  Submit the bug as an Issue here on GitHub, but please make sure to provide enough us information that we can diagnose it:

- **Use a clear and descriptive title** for your Issue so we know what you're talking about.
- **Describe the environment.**  What kind of computer were you using Smile on?  What OS?  What OS version number?  What Smile version number?  What other things were installed that could in any way have influenced the bug?
- **Did it work before, and stop working?**  Did you upgrade anything on your computer or OS?  Did it work in a previous version of the Smile runtime?  Has it always been broken, or is this something that just started happening?
- **Include a sample test case**.  Bug reports without a code sample to demonstrate them will be ignored.  If you can trigger the bug without a code sample, include the exact command-line text required to trigger it instead.
- **Describe what you expected would happen.**  Maybe we think it works just fine as-is:  If what you're seeing isn't right, you need to explain what you think should have happened and why.
- **If this is a crash** — if the Smile runtime segfaulted on you without recovering gracefully — include a crash report with a stack trace.  If an uncaught Smile exception was thrown, include that stack trace instead.

Also, **please be sure to respond** to any of our follow-up questions:  If we ask you additional questions about your Issue and you don't answer for several days, we'll assume you don't care anymore or it's not a real bug, and we'll close the Issue.

### Suggesting Features

> "Art is never finished, only abandoned."
>
> -- Leonardo da Vinci

Before suggesting a new feature, please check the [Issues](https://github.com/seanofw/smile/issues) to see if someone else has already suggested it.  Also, check Google Search and see if your feature is already available as a package you can download and install.

If nobody else has come up with your fancy-schmancy new clever idea, you can submit it as a new [Issue](https://github.com/seanofw/smile/issues) for us to consider.  Please follow these guidelines when you do:

- **Use a clear and descriptive title** for your Issue so we know what you're talking about.
- **Describe what you'd like to see in detail.**  Tell us how your idea is different from how Smile works today, and tell us what should happen.  Don't just write a one-liner; be descriptive, and spell out in detail how you think it should work.
- **Include example code.**  Most feature requests in a programming language are for a new language feature or library, so include example code that shows how the new feature would work and how it would be used.
- **Explain why it's needed.**  Part of the philosophy of Smile is to have as little built into the core language as possible, so that most things can be done in third-party libraries.  A feature change to Smile itself requires some justification:  Explain why it's needed, why current functionality isn't good enough, and try to make a good case for it.
- **Don't expect a "yes."**  There's a long list of things that need to be done in Smile, and your feature, however awesome and cool, might not happen for a while or at all.  Please don't assume that you'll hear a "yes" from the devs just because it's a good idea.

### Contributing Documentation & Examples

Thank you!  There's always room for more documentation and for documentation fixes and for additional examples.

The documentation for Smile, including most of the Smile website, can be found in the `docs/` folder in the Smile repository.

Updating and extending it follows the same basic process as [Contributing Code](#Contributing Code) below:  Fork and clone the repository, make the changes in your fork, commit those changes to your fork, and then issue a pull request to pull your fork back into the main repository.

Most of the documentation can be edited easily, but some parts require a little explanation:

- The introductory book is written using Markdown, just like this document uses.  You'll need to learn Markdown if you want to edit it.
- The formal reference book is written in XML and is compiled to HTML using a C# program (`RefTool`).  If you want to make changes to the formal reference, you'll need to edit the XML, and then you'll need to run the `RefTool` program to generate the compiled HTML.  To do that, you'll need to compile the `RefTool` program.

### Contributing Code

You'd like to contribute a bugfix or feature directly yourself?  That's great!  There are a few things that you should know, though, before you begin:

- **Familiarize yourself with the basic process** of contributing to a Git repository:  Learn how to fork the repo, commit to your fork, and issue a pull request.  We don't have the time to walk you through the basics, but there are ten million tutorials on the Internet that can teach you how to Git and how to GitHub.
- **Make sure you can set up the environment** and build the Smile runtime yourself.  We don't have time to hand-walk you through doing that, but there are some tips at the end of this document.
- **Follow the code standard**.  Your code will become a part of the repository, so it must read like other code in the repository.  The code standard can be found in the [CODE-STANDARD.md](CODE-STANDARD.md) file, right next to this document.  We can and will reject pull requests for code that is mis-formatted, badly named, or has egregious typos and spelling errors in it.
- **Don't assume your pull request will be immediately accepted**.  We have to read and review every pull request carefully to ensure that it follows the spirit of the language, and that it's well-designed, and that doesn't introduce any defects, and that it follows the code standards.  That takes time, and it also means there are a lot of details we're checking for that could prevent your pull request from being accepted.
- **Please use the pull request template**.  No matter how good your code is, your pull request will be rejected if its description is missing or nonsensical.  There's a template that you can use to ensure your pull request contains all the parts we need to know, so please follow it.
- **Use good Git commit messages**.  Your commit messages will become a permanent part of the repository, so they need to be succinct, descriptive, and well-structured:
  - Do not reference an issue number in the first line of the commit message.
  - Use the present tense ("Add feature" not "Added feature").
  - Use the imperative mood ("Move cursor to..." not "Moves cursor to...").
  - Limit the first line to 72 characters or less.
  - Reference issues and pull requests liberally after the first line.
  - After the first line, for complex, large commits, liberally describe the commit's purpose and what it affects.



## What should I know to debug a bug or add a feature?

The core Smile runtime is a single large repository, written in C.  You can clone it from GitHub via Git.  It's known to build in Visual Studio 2017 and under Linux (gcc 4.x).  You'll need either a working copy of Visual Studio (not the Community Edition) or a working GCC under Linux, with associated build tools.  Both versions require Perl to build.

A lot of the code in the Smile runtime is our own, but there are a few third-party packages embedded within the Smile repository itself:

- [Intel Decimal Floating-Point Math Library](https://software.intel.com/en-us/articles/intel-decimal-floating-point-math-library)
- [Boehm Garbage Collector](http://www.hboehm.info/gc/)
- [Oniguruma Regular Expression Library](https://github.com/kkos/oniguruma)

These libraries are directly embedded into the compiled `SmileLib.dll` or `libsmile.so` so as to avoid runtime versioning issues.

There are unit tests that exist to help demonstrate the correctness of the compiled build.  Make sure the unit tests pass on your local copy before doing anything else.

