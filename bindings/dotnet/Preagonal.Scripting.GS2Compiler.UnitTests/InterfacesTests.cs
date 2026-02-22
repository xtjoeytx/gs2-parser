using Xunit;
using Xunit.Abstractions;

namespace Preagonal.Scripting.GS2Compiler.UnitTests;

public class InterfacesTests(ITestOutputHelper testOutputHelper)
{
	[Fact]
	public void Given_script_that_is_faulty_When_compiling_Then_success_is_false_and_error_message_is_returned()
	{
		//Arrange
		const string scriptText =
			"""
						//#CLIENTSIDE
						function onCreated() 
						}
			""";


		//Act
		var result = Interface.CompileCode(scriptText);

		//Assert
		Assert.False(result.Success);
		Assert.Equal("malformed input at line 3: \t\t\t}\n", result.ErrMsg);
	}
	
	[Fact]
	public void Given_script_that_is_correct_When_compiling_Then_success_is_true_and_bytecode_is_not_empty()
	{
		//Arrange
		const string scriptText =
			"""
						//#CLIENTSIDE
						function onCreated() {
						}
			""";


		//Act
		var result = Interface.CompileCode(scriptText);

		//Assert
		Assert.True(result.Success);
		Assert.NotEmpty(result.ByteCode);
	}
	
	[Fact]
	public void Given_script_that_is_correct_When_compiling_without_header_Then_success_is_true_and_bytecode_is_not_empty()
	{
		//Arrange
		const string scriptText =
			"""
						//#CLIENTSIDE
						function onCreated() {
						}
			""";


		//Act
		var result = Interface.CompileCode(scriptText, withHeader: false);

		//Assert
		Assert.True(result.Success);
		Assert.NotEmpty(result.ByteCode);
	}
}