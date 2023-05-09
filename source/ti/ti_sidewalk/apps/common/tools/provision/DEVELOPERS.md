# Setup Developer Environment

Install Poetry see - https://python-poetry.org/

```
pip3 install --user poetry

poetry install
```

# Coverage reports

## Run unit test coverage

```
poetry run coverage run -m unittest discover tests
```

## Get Coverage report

```
poetry run coverage report


Name                        Stmts   Miss  Cover
-----------------------------------------------
sid_provision/__init__.py       0      0   100%
sid_provision/run.py          316     67    79%
tests/test_mfg_bin.py         102      0   100%
tests/test_mfg_obj.py          85      1    99%
-----------------------------------------------
TOTAL                         503     68    86%

## For html coverage report
poetry run coverage html
```

# Beautify Code

```
poetry run black .
```

# Run style check

```
poetry run flake8 .
```

# Run Static Typing check

```
poetry run mypy sid_provision
```

# Generate requirements from poetry

```
poetry export --without-hashes --format=requirements.txt > requirements.txt
```
