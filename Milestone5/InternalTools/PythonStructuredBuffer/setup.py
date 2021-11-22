from setuptools import find_packages, setup

def main():
    setup(name="PyStructuredBuffer",
            version="1.0.0",
            description="StructuredBuffer implementation in Python",
            author="Prawal Gangwar",
            author_email="prawal@secureailabs.com",
            packages=find_packages(),
            package_data={'PyStructuredBuffer': ['PyStructuredBuffer.so']},
            zip_safe=False)

if __name__ == "__main__":
    main()
