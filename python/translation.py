from utility import *

import re
from pathlib import Path


def set_default_language(directory, language_code):
    """
    Sets default language files by copying from translated versions. Looks for
    files with pattern 'filename.{language_code}.ext' and creates 'filename.ext'.
    """
    dir_path = Path(directory)
    if not dir_path.exists() or not dir_path.is_dir():
        raise ValueError(f"Directory '{directory}' does not exist or is not a directory")

    pattern = re.compile(r'^(.+)\.([a-z]{2})\.([^.]+)$')

    translated_files = []
    for file_path in dir_path.iterdir():
        if file_path.is_file():
            match = pattern.match(file_path.name)
            if match and match.group(2) == language_code:
                translated_files.append(file_path)

    if not translated_files:
        raise ValueError(f"No files found for language code '{language_code}' in '{directory}'")

    for translated_file in translated_files:
        match = pattern.match(translated_file.name)
        if match:
            base_name, lang, extension = match.groups()
            default_name = f"{base_name}.{extension}"
            default_path = dir_path / default_name

            with open(translated_file, 'r', encoding='utf-8') as src:
                content = src.read()

            with open(default_path, 'w', encoding='utf-8') as dst:
                dst.write(content)


@automatic_test_function
def get_available_languages(directory):
    """
    Returns a set of 2-letter language codes found in translated files.
    """
    dir_path = Path(directory)
    if not dir_path.exists() or not dir_path.is_dir():
        raise ValueError(f"Directory '{directory}' does not exist or is not a directory")

    pattern = re.compile(r'^.+\.([a-z]{2})\.[^.]+$')
    languages = set()

    for file_path in dir_path.iterdir():
        if file_path.is_file():
            match = pattern.match(file_path.name)
            if match:
                languages.add(match.group(1))

    return languages
