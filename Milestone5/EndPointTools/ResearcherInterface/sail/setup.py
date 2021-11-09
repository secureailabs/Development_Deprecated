from setuptools import find_packages, setup

setup(name='sail',
      version='0.1',
      description='sail api for confidential enclave computing',
      url='',
      author='Jingwei Zhang@Secure AI Labs',
      author_email='jingwei@secureailabs.com',
      license='',
      packages=find_packages(),
      package_data={'sail': ['SAILPyAPI.so']},
      zip_safe=False)
