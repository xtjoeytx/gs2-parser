use gs2compiler::Gs2Context;

#[test]
fn test_success() {
        let code = r#"
        function onCreated() {
            echo("Hello, world!");
        }
        "#;

        // Create a new Gs2Context
        let context = Gs2Context::new();

        // Compile the code
        let result = context.compile_code(&code);

        // Check if the code compiled successfully
        assert!(result.is_ok());
}

#[test]
fn test_fail() {
        let code = r#"
        function onCreated() {
            echo("Hello, world!")
        }
        "#;

        // Create a new Gs2Context
        let context = Gs2Context::new();

        // Compile the code
        let result = context.compile_code(&code);

        // Check if the code failed to compile
        assert!(result.is_err());
}