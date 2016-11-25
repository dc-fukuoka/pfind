Summary: A simple find command which is parallelized by OpenMP.
Name: pfind
Version: 0.1
Release: 1
License: beerware
Group: Applications/File
URL: https://github.com/dc-fukuoka/pfind
Source0: %{name}-%{version}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Prefix: /usr

%description
A simple parallel find command which is parallelized by OpenMP.

%prep
%setup -q

%configure --prefix=%{_prefix}

%build
make -j

%install
%{__make} install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_prefix}
%doc


%changelog
* Fri Nov 25 2016 Daichi Fukuoka <fukuoka@helios85.kaku.iferc-csc.org> -
- Initial build.
