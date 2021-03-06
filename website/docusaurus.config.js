module.exports = {
  title: 'InFormant',
  tagline: 'The user documentation for InFormant, a software for voice analysis',
  url: 'https://docs.in-formant.app',
  baseUrl: '/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'img/favicon.ico',
  organizationName: 'clo-yunhee', // Usually your GitHub org/user name.
  projectName: 'in-formant', // Usually your repo name.
  themeConfig: {
    navbar: {
      title: 'InFormant Documentation',
      // logo: {
      //   alt: 'InFormant Logo',
      //   src: 'img/logo.svg',
      // },
      items: [
        {
          to: 'docs/',
          activeBasePath: 'docs',
          label: 'Docs',
          position: 'left',
        },
        {
          href: 'https://github.com/clo-yunhee/in-formant',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'More',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/clo-yunhee/in-formant',
            },
            {
              label: 'Discord',
              href: 'https://discord.gg/SE9JaNB',
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} InFormant, Built with Docusaurus.`,
    },
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          // Please change this to your repo.
          editUrl:
            'https://github.com/clo-yunhee/in-formant/edit/docs/website/',
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
};
