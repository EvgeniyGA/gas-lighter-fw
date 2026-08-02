def test_formatting_successful(board):
    match = board.wait_for_regex_in_file(r"Formatting successful!")
    assert match is not None, "Ожидалось 'Formatting successful!' в выводе"

def test_mounted_successful(board):
    match = board.wait_for_regex_in_file(r"FatFS mounted successfully!")
    assert match is not None, "Сообщение о монтировании FatFS не найдено"

def test_create_readme_successful(board):
    match = board.wait_for_regex_in_file(r"Created README.TXT")
    assert match is not None, "Ожидалось 'Created README.TXT' в выводе"





