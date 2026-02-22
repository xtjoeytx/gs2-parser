namespace Preagonal.Scripting.GS2Compiler;

public struct CompilerResponse
{
	public bool    Success;
	public string? ErrMsg;
	public byte[]  ByteCode;
}