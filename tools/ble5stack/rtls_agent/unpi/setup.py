from setuptools import setup
from os import path

this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(name='ti-simplelink-unpi',
      version='0.3',
      description='Serial uNPI frame parser and builder',
      long_description=long_description,
      long_description_content_type='text/markdown',
      url='',
      author='a0132628',
      author_email='a0132628@ti.com',
      license='BSD-3',
      packages=['unpi'],
      zip_safe=False,
      )
