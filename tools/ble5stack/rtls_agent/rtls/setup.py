from setuptools import setup, find_packages
from os import path

this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(name='ti-simplelink-rtls',
      version='0.9',
      description='Wrapper for the uNPI RTLS subsystem',
      long_description=long_description,
      long_description_content_type='text/markdown',
      url='',
      author='a0132628',
      author_email='a0132628@ti.com',
      license='BSD-3',
      packages=find_packages(),
      zip_safe=False,
      )
